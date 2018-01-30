;-----------------------------------------------------------
;实验一示例程序                                            |
;                                                          |
;功能：读取开关量的状态取反后送到LED显示                   |
;74LS244带有8位拨位开关，地址为80H or 82H or 84H or 86H    |
;74LS273带有8个发光二极管，地址为88H or 8AH or 8CH or 8EH  |
;                                                          |
;Designer：LMC                                             |
;-----------------------------------------------------------
		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集
;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
;
PortIn	EQU	80h	;定义输入端口号
PortOut	EQU	88h	;定义输出端口号
;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段
;MyVar	DB	?					; 定义MyVar变量   


;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; 定义汇编程序执行入口点
;
;以下开始放置用户指令代码
;

;-----------------------------------------------------------
;主程序部分,读取开关量状态取反后送显示                     |
;-----------------------------------------------------------

Again:
		MOV CX,03fffh
RedRed:         MOV AL,36h
		OUT PortOut,AL
		DEC CX
		JNZ RedRed
LPE:		
		MOV CX,0ffffh
GreenRed:       MOV AL,33h
		OUT PortOut,AL
		DEC CX
		JNZ GreenRed
		
		MOV CX,0fffh
SNGreenOff:     MOV AL,37h
		OUT PortOut,AL
		DEC CX
		JNZ SNGreenOff
		
		MOV CX,0fffh
SNGreenOn:      MOV AL,33h
		OUT PortOut,AL
		DEC CX
		JNZ SNGreenOn
		
		MOV CX,03fffh
YellowRed:      MOV AL,35h
		OUT PortOut,AL
		DEC CX
		JNZ YellowRed
		
		MOV CX,0ffffh
RedGreen:       MOV AL,1Eh
		OUT PortOut,AL
		DEC CX
		JNZ RedGreen
		
		MOV CX,0fffh
WEGreenOff:     MOV AL,3Eh
		OUT PortOut,AL
		DEC CX
		JNZ WEGreenOff
		
		MOV CX,0fffh
WEGreenOn:      MOV AL,1Eh
		OUT PortOut,AL
		DEC CX
		JNZ WEGreenOn
		
		MOV CX,03fffh
RedYellow:      MOV AL,2Eh
		OUT PortOut,AL
		DEC CX
		JNZ RedYellow
		
		JMP LPE

		END