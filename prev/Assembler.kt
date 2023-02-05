package eyre

import core.memory.NativeWriter
import eyre.Width.*

class Assembler(private val srcSet: SrcSet) {


	private val dataWriter = NativeWriter()

	private val textWriter = NativeWriter()

	private var writer = textWriter

	private val relocations = ArrayList<Relocation>()

	private fun error(): Nothing = error("Invalid encoding")

	private val groups = EncodingReader().read()

	private var bssSize = 0

	private var rexRequired = false

	private var rexDisallowed = false

	private lateinit var group: InstructionGroup



	fun assemble(): AssemblerOutput {
		for(srcFile in srcSet.srcFiles)
			assemble(srcFile.nodes)

		return AssemblerOutput(
			textWriter.getTrimmedBytes(),
			dataWriter.getTrimmedBytes(),
			bssSize,
			relocations
		)
	}



	private fun assemble(nodes: List<AstNode>) {
		for(node in nodes) {
			when(node) {
				is ProcNode       -> handleProc(node)
				is LabelNode      -> handleLabel(node)
				is VarNode        -> handleVar(node)
				is InsNode        -> handleInstruction(node)
				is ResNode        -> handleRes(node)
				is SizeofNode,
				is NamespaceNode,
				is ScopeEndNode,
				is EnumNode,
				is ImportNode,
				is ConstNode      -> { }
				else              -> error("Invalid node: $node")
			}
		}
	}



	private fun handleInstruction(node: InsNode) {
		rexRequired = false
		rexDisallowed = false

		group = groups[node.mnemonic]
			?: error("No encodings for mnemonic: ${node.mnemonic}")

		val customEncoding = customEncodings[node.mnemonic]

		if(customEncoding != null)
			customEncoding(node)
		else
			assemble(node)
	}



	private fun handleLabel(node: LabelNode) {
		node.symbol.pos = writer.pos
	}



	private fun handleProc(node: ProcNode) {
		node.symbol.pos = writer.pos
	}



	private fun handleVar(node: VarNode) {
		dataWriter.align8()

		node.symbol.pos = dataWriter.pos
		for(part in node.parts) {
			for(value in part.values) {
				if(value is StringNode) {
					for(char in value.value.string)
						dataWriter.int(part.width, char.code)
				} else {
					dataWriter.int(part.width, resolveImm(value))
				}
			}
		}
	}



	private fun handleRes(node: ResNode) {
		val size = node.symbol.size

		val alignment = when(size) {
			1    -> 1
			2    -> 2
			3    -> 4
			4    -> 4
			else -> 8
		}

		bssSize = (bssSize + alignment - 1) and -alignment
		node.symbol.pos = bssSize
		bssSize += size
	}



	/*
	Resolution
	 */



	private var baseReg: Register? = null

	private var indexReg: Register? = null

	private var indexScale = 0

	private var aso = false

	private var memRefCount = 0

	private val hasMemReloc get() = memRefCount > 0

	private var importSymbol: Symbol? = null

	private var immRefCount = 0

	private val hasImmReloc get() = immRefCount > 0



	private fun resolveMem(root: AstNode): Int {
		baseReg      = null
		indexReg     = null
		indexScale   = 0
		aso          = false
		importSymbol = null
		memRefCount  = 0

		val disp = resolveMemRec(if(root is ImmNode) root.value else root, 1)

		if(baseReg != null) {
			if(indexReg != null) {
				if(baseReg!!.width != indexReg!!.width)
					error()
				if(baseReg!!.width.is32)
					aso = true
				else if(baseReg!!.width.isNot64)
					error()

				if(indexReg!!.isSP) {
					if(indexScale != 1) error()
					val temp = indexReg
					indexReg = baseReg
					baseReg = temp
				}
			} else {
				if(baseReg!!.width.is32)
					aso = true
				else if(baseReg!!.width.isNot64)
					error()
			}
		}

		if(importSymbol != null && memRefCount != 1) error()

		if(!hasImmReloc && !disp.isImm32) error()

		return disp.toInt()
	}



	private fun resolveStringImm(string: String): Long {
		var value = 0L
		if(string.length > 8) error("String literal out of range")
		for((i, c) in string.withIndex())
			if(c.code > 255)
				error("String literal char out of range: $c")
			else
				value = value or (c.code.toLong() shl (i shl 3))
		return value
	}



	private fun resolveImm(root: AstNode): Long {
		immRefCount = 0
		return if(root is ImmNode)
			root.value.resolveImmRec()
		else
			root.resolveImmRec()
	}



	private fun AstNode.resolveImmRec(): Long = when(this) {
		is UnaryNode     -> op.calculate(node.resolveImmRec())
		is IntNode       -> value
		is StringNode    -> resolveStringImm(value.string)
		is DotNode       -> (right.symbol as? IntSymbol)?.value ?: error()
		is BinaryNode    -> op.calculate(left.resolveImmRec(), right.resolveImmRec())
		is SymNode       -> when(val symbol = this.symbol) {
			is IntSymbol -> symbol.value
			is DllSymbol -> error("Cannot reference DLL imports in immediates")
			is Ref       -> { immRefCount++; 0 }
			else         -> error("Invalid symbol: $symbol, ${this.printableString}")
		}
		is SizeofNode    -> size
		else             -> error("Invalid immediate node: $this")
	}



	private fun resolveMemRec(node: AstNode, positivity: Int): Long {
		if(node is UnaryNode)
			return node.op.calculate(resolveMemRec(node.node, positivity * node.op.positivity))

		if(node is IntNode)
			return node.value

		if(node is StringNode)
			return resolveStringImm(node.value.string)

		if(node is RegNode) {
			if(positivity <= 0) error()

			if(baseReg != null) {
				if(indexReg != null) error()
				indexReg = node.value
				indexScale = 1
			} else {
				baseReg = node.value
			}

			return 0
		}

		if(node is BinaryNode) {
			if(node.op == BinaryOp.MUL) {
				if(node.left is RegNode && node.right is IntNode) {
					if(indexReg != null || positivity <= 0) error()
					indexReg = node.left.value
					indexScale = node.right.value.toInt()
					return 0
				} else if(node.left is IntNode && node.right is RegNode) {
					if(indexReg != null || positivity <= 0) error()
					indexReg = node.right.value
					indexScale = node.left.value.toInt()
					return 0
				}
			}

			return node.op.calculate(
				resolveMemRec(node.left, positivity * node.op.leftPositivity),
				resolveMemRec(node.right, positivity * node.op.rightPositivity)
			)
		}

		if(node is SymNode) {
			val symbol = node.symbol

			if(symbol is IntSymbol)
				return symbol.value

			if(symbol is DllSymbol) {
				memRefCount++
				importSymbol = symbol
				return 0
			}

			if(symbol is Ref) {
				memRefCount++
				return 0
			}

			error("Invalid mem symbol: $symbol")
		}

		error()
	}



	private fun addImmReloc(node: AstNode, width: Width) {
		relocations.add(Relocation(
			section  = Section.TEXT,
			position = writer.pos,
			width    = width,
			value    = node,
			base     = null
		))
	}



	private fun addRelReloc(node: AstNode, width: Width, offset: Int = 0) {
		relocations.add(Relocation(
			section  = Section.TEXT,
			position = writer.pos,
			width    = width,
			value    = node,
			base     = Ref(Section.TEXT, writer.pos + width.bytes + offset)
		))
	}



	private fun encoding(operands: Operands) =
		if(operands !in group)
			error("Invalid encoding")
		else
			group.instructions[(group.operandsBits and (operands.bit - 1)).countOneBits()]




	/*
	Base encoding
	 */



	private fun writeOpReg(opcode: Int, offset: Int) {
		val length = ((39 - (opcode or 1).countLeadingZeroBits()) and -8) shr 3
		writer.int(opcode + (offset shl ((length - 1) shl 3)))
	}



	private fun writeModRM(mod: Int, reg: Int, rm: Int) {
		writer.i8((mod shl 6) or (reg shl 3) or rm)
	}



	private fun writeSib(scale: Int, index: Int, base: Int) {
		writer.i8((scale shl 6) or (index shl 3) or base)
	}



	private fun writeRex(w: Int, r: Int, x: Int, b: Int) {
		val value = 0b0100_0000 or (w shl 3) or (r shl 2) or (x shl 1) or b

		if(rexRequired || value != 0b0100_0000)
			if(rexDisallowed)
				error()
			else
				writer.i8(value)
	}



	private fun writeMem(
		opcode    : Int,
		node      : AstNode,
		rexW      : Int,
		rexR      : Int,
		reg       : Int,
		immLength : Int
	) {
		val disp   = resolveMem(node)
		val base   = baseReg
		val index  = indexReg
		val scale  = indexScale

		if(aso) writer.i8(0x67)

		val mod = when {
			hasMemReloc  -> 2
			disp == 0    -> if(base != null && base.value == 5) 1 else 0
			disp.isImm8  -> 1
			else         -> 2
		}

		fun relocAndDisp() {
			if(hasMemReloc) {
				relocations.add(Relocation(Section.TEXT, writer.pos, BIT32, node, null))
				writer.i32(0)
			} else if(mod == 0b01)
				writer.i8(disp)
			else if(mod == 0b10)
				writer.i32(disp)
		}

		if(index != null) { // SIB
			if(scale.countOneBits() != 1) error()
			val finalScale = scale.countTrailingZeroBits()

			if(base != null) {
				writeRex(rexW, rexR, index.rex, base.rex)
				writer.int(opcode)
				writeModRM(mod, reg, 0b100)
				writeSib(finalScale, index.value, base.value)
				relocAndDisp()
			} else {
				writeRex(rexW, rexR, index.rex, 0)
				writer.int(opcode)
				writeModRM(0, reg, 0b100)
				writeSib(finalScale, index.value, 0b101)
				relocAndDisp()
			}
		} else if(base != null) { // Indirect
			writeRex(rexW, rexR, 0, base.rex)
			writer.int(opcode)

			if(base.isSP) {
				writeModRM(mod, reg, 0b100)
				writeSib(0, 0b100, 0b100)
			} else {
				writeModRM(mod, reg, base.value)
			}

			relocAndDisp()
		} else if(memRefCount == 1) { // RIP-relative
			writeRex(rexW, rexR, 0, 0)
			writer.int(opcode)
			writeModRM(0b00, reg, 0b101)
			relocations.add(Relocation(Section.TEXT, writer.pos, BIT32, node, Ref(Section.TEXT, writer.pos + 4 + immLength)))
			writer.advance(4)
		} else if(mod != 0) { // Absolute 32-bit
			writeRex(rexW, rexR, 0, 0)
			writer.int(opcode)
			writeModRM(0b00, reg, 0b100)
			writeSib(0b00, 0b100, 0b101)
			relocAndDisp()
		} else { // Empty memory operand
			error()
		}
	}



	private fun writeRel8(node: ImmNode, value: Long) {
		if(hasImmReloc) {
			addRelReloc(node, BIT8)
			writer.advance(1)
		} else {
			if(!value.isImm8) error()
			writer.i8(value.toInt())
		}
	}



	private fun writeRel32(node: ImmNode, value: Long) {
		if(hasImmReloc) {
			addRelReloc(node, BIT32)
			writer.advance(4)
		} else {
			if(value.isImm32) error()
			writer.i32(value.toInt())
		}
	}



	private fun writeImmInternal(node: ImmNode, width: Width, value: Long) {
		if(hasImmReloc) {
			addImmReloc(node, width)
			writer.advance(width.bytes)
			return
		}

		if(value !in width) error()

		when(width) {
			BIT8  -> writer.i8(value.toInt())
			BIT16 -> writer.i16(value.toInt())
			BIT32 -> writer.i32(value.toInt())
			BIT64 -> writer.i64(value)
			else  -> error()
		}
	}



	private fun writeImm(node: ImmNode, width: Width, value: Long)
		= writeImmInternal(node, if(width.is64) BIT32 else width, value)



	/*
	Encoding
	 */



	private fun encodeNone(operands: Operands) {
		val encoding = encoding(operands)
		if(encoding.prefix != 0) writer.i8(encoding.prefix)
		writer.int(encoding.opcode)
	}



	/**
	 * Encodes a register in ModRM:RM.
	 */
	private fun encode1R(operands: Operands, op1: Register) {
		val encoding = encoding(operands)
		val width = op1.width
		if(width !in encoding.widths) error()
		if(width == BIT16) writer.i8(0x66)
		rexRequired = op1.rex8
		rexDisallowed = op1.noRex8
		writeRex(width.rexW and encoding.widths.rexMod, 0, 0, op1.rex)
		writer.int(encoding.opcode + (encoding.widths.bits and width.opcodeOffset))
		writeModRM(0b11, encoding.extension, op1.value)
	}



	/**
	 * Encodes a register in the opcode register field.
	 */
	private fun encode1O(operands: Operands, op1: Register) {
		val encoding = encoding(operands)
		val width = op1.width
		if(width !in encoding.widths) error()
		if(width.is16) writer.i8(0x66)
		rexRequired = op1.rex8
		rexDisallowed = op1.noRex8
		writeRex(width.rexW and encoding.widths.rexMod, 0, 0, op1.rex)
		writer.int(encoding.opcode + (encoding.widths.bits and width.opcodeOffset))
		writeOpReg(encoding.opcode + ((encoding.widths.bits and width.opcodeOffset) shl 3), op1.value)
	}



	/**
	 * Encodes a memory operand
	 */
	private fun encode1M(operands: Operands, op1: MemNode, immLength: Int) {
		val encoding = encoding(operands)
		val width = op1.width ?: error()
		if(width !in encoding.widths) error()
		if(width.is16) writer.i8(0x66)
		writeMem(
			encoding.opcode + (encoding.widths.bits and width.opcodeOffset),
			op1.value,
			width.rexW and encoding.widths.rexMod,
			0,
			encoding.extension,
			immLength
		)
	}



	/**
	 * Encodes 2 registers in ModRM:RM and ModRM:REG respectively
	 */
	private fun encode2RR(operands: Operands, op1: Register, op2: Register) {
		val encoding = encoding(operands)
		val width = op1.width
		if(width !in encoding.widths) error()
		if(width.is16) writer.i8(0x66)
		rexRequired = op1.rex8 || op2.rex8
		rexDisallowed = op1.noRex8 || op2.noRex8
		writeRex(width.rexW and encoding.widths.rexMod, op2.rex, 0, op1.rex)
		writer.int(encoding.opcode + (width.opcodeOffset and encoding.widths.bits))
		writeModRM(0b11, op2.value, op1.value)
	}




	/**
	 * Encodes a register operand and a memory operand in ModRM:REG and ModRM:RM respectively.
	 */
	private fun encode2RM(operands: Operands, op1: Register, op2: MemNode, immLength: Int) {
		val encoding = encoding(operands)
		val width = op1.width
		if(width !in encoding.widths) error()
		if(width.is16) writer.i8(0x66)
		rexRequired = op1.rex8
		rexDisallowed = op1.noRex8
		writeMem(
			encoding.opcode + (encoding.widths.bits and width.opcodeOffset),
			op2.value,
			width.rexW and encoding.widths.rexMod,
			op1.rex,
			op1.value,
			immLength
		)
	}



	/*
	Assembly
	 */



	private fun assemble(node: InsNode) {
		when {
			node.op1 == null -> assemble0()
			node.op2 == null -> assemble1(node)
			node.op3 == null -> if(node.op1 is RegNode) assemble2R(node) else assemble2M(node)
			node.op4 == null -> error()
			else             -> error()
		}
	}



	private fun assemble0() {
		encodeNone(Operands.NONE)
	}


	private fun assemble1(node: InsNode) {
		val op1 = node.op1

		if(op1 is RegNode) {
			if(Specifier.O in group) {
				encode1O(Operands.O, op1.value)
			} else {
				encode1R(Operands.R, op1.value)
			}
		} else if(op1 is MemNode) {
			encode1M(Operands.M, op1, 0)
		} else if(op1 is ImmNode) {
			val imm = resolveImm(op1)

			if(Specifier.REL8 in group && ((!hasImmReloc && node.shortImm) || Specifier.REL32 !in group)) {
				encodeNone(Operands.REL8)
				writeRel8(op1, imm)
			} else if(Specifier.REL32 in group) {
				encodeNone(Operands.REL32)
				writeRel32(op1, imm)
			} else if(Specifier.I8 in group && ((!hasImmReloc && imm.isImm8) || (Specifier.I16 !in group && Specifier.I32 !in group))) {
				encodeNone(Operands.I8)
				writeImm(op1, BIT8, imm)
			} else if(Specifier.I16 in group && ((!hasImmReloc && imm.isImm16) || Specifier.I32 !in group)) {
				encodeNone(Operands.I16)
				writeImm(op1, BIT16, imm)
			} else {
				encodeNone(Operands.I32)
				writeImm(op1, BIT32, imm)
			}
		} else {
			error()
		}
	}



	private fun assemble2R(node: InsNode) {
		val op1 = (node.op1 as RegNode).value
		val op2 = node.op2
		val width = op1.width

		if(op2 is RegNode) {
			val r2 = op2.value
			val width2 = r2.width

			if(width != width2) {
				if(Specifier.RM_CL in group && r2 == Register.CL) {
					encode1R(Operands.RM_CL, op1)
				} else {
					error()
				}
			} else {
				encode2RR(Operands.R_R, op1, r2)
			}
		} else if(op2 is MemNode) {
			val width2 = op2.width

			if(width2 != null && width2 != width)
				error()

			encode2RM(Operands.R_M, op1, op2, 0)
		} else if(op2 is ImmNode) {
			val imm = resolveImm(op2)

			if(Specifier.RM_I8 in group && !hasImmReloc && width.isNot8 && imm.isImm8) {
				encode1R(Operands.R_I8, op1)
				writeImm(op2, BIT8, imm)
			} else if(Specifier.RM_1 in group && !hasImmReloc && imm == 1L) {
				encode1R(Operands.RM_1, op1)
			} else if(op1.isA && Specifier.A_I in group) {
				encodeNone(Operands.A_I)
				writeImm(op2, width, imm)
			} else {
				encode1R(Operands.R_I, op1)
				writeImm(op2, width, imm)
			}
		} else {
			error()
		}
	}



	private fun assemble2M(node: InsNode) {
		val op1 = node.op1 as MemNode
		val op2 = node.op2!!

		if(op2 is RegNode) {
			if(Specifier.RM_CL in group && op2.value == Register.CL) {
				encode1M(Operands.RM_CL, op1, 0)
			} else {
				if(op1.width != null && op1.width != op2.value.width) error()
				encode2RM(Operands.M_R, op2.value, op1, 0)
			}
		} else if(op2 is ImmNode) {
			val width = op1.width ?: error()
			val imm = resolveImm(op2)
			// Issue with order of resolution with IMM and MEM.
			if(Specifier.RM_I8 in group && !hasImmReloc && width.isNot8 && imm.isImm8) {
				encode1M(Operands.M_I8, op1, 1)
				writeImm(op2, BIT8, imm)
			} else if(Specifier.RM_1 in group && !hasImmReloc && imm == 1L) {
				encode1M(Operands.RM_1, op1, 0)
			} else {
				encode1M(Operands.M_I, op1, width.immLength)
				writeImm(op2, width, imm)
			}
		} else {
			error()
		}
	}



	private val customEncodings: Map<Mnemonic, (InsNode) -> Unit> = mapOf(
		Mnemonic.IMUL   to ::customEncodeIMUL,
		Mnemonic.XCHG   to ::customEncodeXCHG,
		Mnemonic.MOV    to ::customEncodeMOV,
		Mnemonic.MOVSX  to ::customEncodeMOVSX,
		Mnemonic.MOVZX  to ::customEncodeMOVZX,
		Mnemonic.MOVSXD to ::customEncodeMOVSXD,
		Mnemonic.IN     to ::customEncodeIN,
		Mnemonic.OUT    to ::customEncodeOUT
	)



	/**
	 * E6    OUT  I8_AL
	 * E766  OUT  I8_AX
	 * E7    OUT  I8_EAX
	 * EE    OUT  DX_AL
	 * EF66  OUT  DX_AX
	 * EF    OUT  DX_EAX
	 */
	private fun customEncodeOUT(node: InsNode) {
		if(node.op3 != null) error()
		if(node.op2 !is RegNode) error()
		val op2 = node.op2.value

		if(!op2.isA) error()

		if(node.op1 is RegNode) {
			if(node.op1.value != Register.DX) error()
			when(op2.width) {
				BIT8  -> writer.i8(0xEE)
				BIT16 -> writer.i16(0xEF66)
				BIT32 -> writer.i8(0xEF)
				else  -> error()
			}
		} else if(node.op1 is ImmNode) {
			val imm = resolveImm(node.op1)
			when(op2.width) {
				BIT8  -> writer.i8(0xE6)
				BIT16 -> writer.i16(0xE766)
				BIT32 -> writer.i8(0xE7)
				else  -> error()
			}
			if(hasImmReloc) addImmReloc(node.op1, BIT8)
			else if(!imm.isImm8) error()
			writer.i8(imm.toInt())
		} else {
			error()
		}
	}



	/**
	 * E4    IN  AL_I8
	 * E566  IN  AX_IMM8
	 * E5    IN  EAX_IMM8
	 * EC    IN  AL_DX
	 * ED66  IN  AX_DX
	 * ED    IN  EAX_DX
	 */
	private fun customEncodeIN(node: InsNode) {
		if(node.op3 != null) error()
		if(node.op1 !is RegNode) error()
		val op1 = node.op1.value

		if(!op1.isA) error()

		if(node.op2 is RegNode) {
			if(node.op2.value != Register.DX) error()
			when(op1.width) {
				BIT8  -> writer.i8(0xEC)
				BIT16 -> writer.i16(0xED66)
				BIT32 -> writer.i8(0xED)
				else  -> error()
			}
		} else if(node.op2 is ImmNode) {
			val imm = resolveImm(node.op2)
			when(op1.width) {
				BIT8  -> writer.i8(0xE4)
				BIT16 -> writer.i16(0xE566)
				BIT32 -> writer.i8(0xE5)
				else  -> error()
			}
			if(hasImmReloc) addImmReloc(node.op2, BIT8)
			else if(!imm.isImm8) error()
			writer.i8(imm.toInt())
		} else {
			error()
		}
	}



	/**
	 *     BE0F  MOVSX   R_RM8   NO8
	 *     BF0F  MOVSX   R_RM16  NO816
	 */
	private fun customEncodeMOVSX(node: InsNode) {
		if(node.op3 != null) error()
		if(node.op1 !is RegNode) error()

		if(node.op2 is RegNode) {
			when(node.op2.value.width) {
				BIT8  -> encode2RR(Operands.CUSTOM1, node.op1.value, node.op2.value)
				BIT16 -> encode2RR(Operands.CUSTOM2, node.op1.value, node.op2.value)
				else  -> error()
			}
		} else if(node.op2 is MemNode) {
			when(node.op2.width) {
				BIT8  -> encode2RM(Operands.CUSTOM1, node.op1.value, node.op2, 0)
				BIT16 -> encode2RM(Operands.CUSTOM1, node.op1.value, node.op2, 0)
				else  -> error()
			}
		} else {
			error()
		}
	}



	/**
	 *     B60F  MOVZX   R_RM8   NO8
	 *     B70F  MOVZX   R_RM16  NO816
	 */
	private fun customEncodeMOVZX(node: InsNode) {
		if(node.op3 != null) error()
		if(node.op1 !is RegNode) error()

		if(node.op2 is RegNode) {
			when(node.op2.value.width) {
				BIT8  -> encode2RR(Operands.CUSTOM1, node.op1.value, node.op2.value)
				BIT16 -> encode2RR(Operands.CUSTOM2, node.op1.value, node.op2.value)
				else  -> error()
			}
		} else if(node.op2 is MemNode) {
			when(node.op2.width) {
				BIT8  -> encode2RM(Operands.CUSTOM1, node.op1.value, node.op2, 0)
				BIT16 -> encode2RM(Operands.CUSTOM1, node.op1.value, node.op2, 0)
				else  -> error()
			}
		} else {
			error()
		}
	}



	/**
	 *     63    MOVSXD  R16_RM16  OSO
	 *     63    MOVSXD  R32_RM32
	 *     63    MOVSXD  R64_RM32  REX.W
	 */
	private fun customEncodeMOVSXD(node: InsNode) {
		if(node.op3 != null) error()
		if(node.op1 !is RegNode) error()
		val op1 = node.op1.value
		val width = op1.width

		if(node.op2 is RegNode) {
			val op2 = node.op2.value

			when {
				width.is16 && op2.width.is16 -> { }
				width.is32 && op2.width.is32 -> { }
				width.is64 && op2.width.is32 -> { }
				else                         -> error()
			}

			encode2RR(Operands.CUSTOM1, op1, op2)
		} else if(node.op2 is MemNode) {
			val op2 = node.op2

			when {
				op2.width == null            -> { }
				width.is16 && op2.width.is16 -> { }
				width.is32 && op2.width.is32 -> { }
				width.is64 && op2.width.is32 -> { }
				else                         -> error()
			}

			encode2RM(Operands.CUSTOM1, op1, op2, 0)
		} else {
			error()
		}
	}



	private fun customEncodeIMUL(node: InsNode) {
		if(node.op2 == null || node.op3 == null) assemble(node)
		if(node.op4 != null) error()

		if(node.op1 !is RegNode || node.op3 !is ImmNode) error()

		val op1 = node.op1.value
		val op3 = node.op3
		val width = op1.width
		val imm = resolveImm(node.op3)

		if(node.op2 is RegNode) {
			val op2 = node.op2.value

			if(width != op2.width) error()

			if(!hasImmReloc && imm.isImm8) {
				encode2RR(Operands.CUSTOM1, op1, op2)
				writeImm(op3, BIT8, imm)
			} else {
				encode2RR(Operands.CUSTOM2, op1, op2)
				writeImm(op3, width, imm)
			}
		} else if(node.op2 is MemNode) {
			val op2 = node.op2

			if(op2.width != null && op2.width != width) error()

			if(!hasImmReloc && imm.isImm8) {
				encode2RM(Operands.CUSTOM1, op1, op2, 1)
				writeImm(op3, BIT8, imm)
			} else {
				encode2RM(Operands.CUSTOM2, op1, op2, width.immLength)
				writeImm(op3, width, imm)
			}
		} else {
			error()
		}
	}



	private fun customEncodeMOV(node: InsNode) {
		if(node.op3 != null) error()

		if(node.op1 is RegNode && node.op2 is ImmNode) {
			val imm = resolveImm(node.op2)

			if(hasImmReloc || !imm.isImm32) {
				encode1O(Operands.CUSTOM1, node.op1.value)
				writeImmInternal(node.op2, node.op1.value.width, imm)
			} else
				assemble(node)
		} else
			assemble(node)
	}



	private fun customEncodeXCHG(node: InsNode) {
		when {
			node.op3 != null        -> error()
			node.op1 !is RegNode ||
			node.op2 !is RegNode    -> assemble(node)
			node.op1.value.isA      -> encode1O(Operands.CUSTOM1, node.op2.value)
			node.op2.value.isA      -> encode1O(Operands.CUSTOM1, node.op1.value)
			else                    -> assemble(node)
		}
	}


}