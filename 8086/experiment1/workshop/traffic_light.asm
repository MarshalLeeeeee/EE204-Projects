;-----------------------------------------------------------
;ʵ��һʾ������                                            |
;                                                          |
;���ܣ���ȡ��������״̬ȡ�����͵�LED��ʾ                   |
;74LS244����8λ��λ���أ���ַΪ80H or 82H or 84H or 86H    |
;74LS273����8����������ܣ���ַΪ88H or 8AH or 8CH or 8EH  |
;                                                          |
;Designer��LMC                                             |
;-----------------------------------------------------------
		.MODEL	SMALL			; �趨8086������ʹ��Small model
		.8086				; �趨����8086���ָ�
;-----------------------------------------------------------
;	���Ŷ���                                               |
;-----------------------------------------------------------
;
PortIn	EQU	80h	;��������˿ں�
PortOut	EQU	88h	;��������˿ں�
;-----------------------------------------------------------
;	�����ջ��                                             |
;-----------------------------------------------------------
		.stack 100h				; ����256�ֽ������Ķ�ջ

;-----------------------------------------------------------
;	�������ݶ�                                             |
;-----------------------------------------------------------
		.data					; �������ݶ�
;MyVar	DB	?					; ����MyVar����   


;-----------------------------------------------------------
;	��������                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; ���������ִ����ڵ�
;
;���¿�ʼ�����û�ָ�����
;

;-----------------------------------------------------------
;�����򲿷�,��ȡ������״̬ȡ��������ʾ                     |
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