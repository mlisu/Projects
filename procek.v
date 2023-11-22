module clock(c, c2);
input c;
reg [25:0] l = 1;
output c2;
assign c2 = !l;
always @(posedge c) begin
	if (l == 2) l <= 0;
	else l <= l+1;
end
endmodule

module rom(clk, clk2, addr, instr);
input clk, clk2;
input [4:0] addr;
output [15:0] instr;
reg [15:0] instr = {4'd10,4'd0,8'd0};
always @(posedge clk) begin
		if (clk2) begin
			case (addr)	

				//wpisywanie do rejestrów 1-5 liczb 1-5
				0: instr <= {4'b0000,4'd1,4'd1,4'd1};
				1: instr <= {4'b0000,4'd2,4'd1,4'd1};
				2: instr <= {4'b0000,4'd3,4'd2,4'd1};
				3: instr <= {4'b0000,4'd4,4'd3,4'd1};
				4: instr <= {4'b0000,4'd5,4'd4,4'd1};
				
				//test instr 1-7
				5: instr <= {4'd1,4'd6,4'd1,4'd2};
				6: instr <= {4'd4,4'd7,4'd5,4'd2};
				7: instr <= {4'd5,4'd8,4'd5,4'd3};
				8: instr <= {4'd6,4'd9,4'd5,4'd2};
				9: instr <= {4'd7,4'd10,4'd4,4'd3};
				//skok o 3 instrukcje, instr. 11 się wykona
				10: instr <= {4'b1111,12'd3};
				11: instr <= {4'd8,4'd11,4'd0,4'd1}; 
				12: instr <= {4'd10,4'd0,8'd0}; // nie wykona sie
				
				13: instr <= {4'd8,4'd12,4'd2,4'd3};
				14: instr <= {4'd9,4'd3,4'd4,4'd9}; //pisze do rej 13
				
				//wypisywanie wartosci rejestrow do ktorych cos pisano
				15: instr <= {4'b0000,4'd6,4'd6,4'd0};
				16: instr <= {4'b0000,4'd7,4'd7,4'd0};
				17: instr <= {4'b0000,4'd8,4'd8,4'd0};
				18: instr <= {4'b0000,4'd9,4'd9,4'd0};
				19: instr <= {4'b0000,4'd10,4'd10,4'd0};
				20: instr <= {4'b0000,4'd11,4'd11,4'd0};
				21: instr <= {4'b0000,4'd12,4'd12,4'd0};
				22: instr <= {4'b0000,4'd13,4'd13,4'd0};

		
				default: instr <= {4'd10,4'd0,8'd0};
			endcase
		end
end
endmodule

/* //Symulacja pamięci do testów - kod od prowadzącego
module RAM16X1S(O, A0, A1, A2, A3, D, WCLK, WE);

parameter [15:0] INIT = 16'h0000;

input D;		// Data In
input WE;		// Write Enable for port A
input WCLK;		// Clock for port A
input A0;		// Address for port A, bit 0
input A1;		// Address for port A, bit 1
input A2;		// Address for port A, bit 2
input A3;		// Address for port A, bit 3
output O;		// port A data out

wire [3:0] A;

assign A = { A3, A2, A1, A0 };

reg dp[0:15];

assign O = dp[A];

integer i;
initial begin
	dp[0] = INIT[0];
	dp[1] = INIT[1];
	dp[2] = INIT[2];
	dp[3] = INIT[3];
	dp[4] = INIT[4];
	dp[5] = INIT[5];
	dp[6] = INIT[6];
	dp[7] = INIT[7];
	dp[8] = INIT[8];
	dp[9] = INIT[9];
	dp[10] = INIT[10];
	dp[11] = INIT[11];
	dp[12] = INIT[12];
	dp[13] = INIT[13];
	dp[14] = INIT[14];
	dp[15] = INIT[15];
end

always @(posedge WCLK) begin
		if (WE) dp[A] <= D;
end

endmodule
*/

module cpu(clk, clk2, count, instr, led, DA, DOUT, DIN, RD, WR);
input clk, clk2;
input [15:0] instr;
output [7:0] led;
reg [7:0] led = 0;
output [4:0] count;
reg [4:0] count = 0;

output [15:0] DA;
	reg [15:0] DA =0;
output [15:0] DOUT;
	reg [15:0] DOUT;
input [15:0] DIN;
output RD;
	reg RD = 0;
output WR;
	reg WR = 0;

reg [3:0] DAtempWR;
reg stopPC = 0;
reg stopExe = 0;
reg [3:0] opCode;
reg [3:0] DAtempRD;
reg stopPC2 = 0;

always @(posedge clk) begin
			if (clk2) begin
				//drugi stan dla dodawania stałej i trzeci dla instrukcji 1,4-7,8,9
				if ( RD && ( !stopPC2 || (stopPC && stopPC2) ) ) begin
					RD <= 0; WR <= 1; DA <= DAtempWR;
					stopPC2 = 0;
					case (opCode)	
						0: begin 
							if (DAtempRD[3] == 1) DOUT <= DIN - (~DAtempRD + 1);
							else DOUT <= DIN + DAtempRD;
						end
						1: DOUT <= DOUT + DIN;
						4: DOUT <= DOUT - DIN;
						5: DOUT <= DOUT & DIN;
						6: DOUT <= DOUT | DIN;
						7: DOUT <= DOUT ^ DIN;
						
						8: DOUT <= DIN;
						9: DOUT <= DIN;
						//default: ;
					endcase
				end else begin
					stopExe = 0;
					WR <= 0;
				end
				
				//pierwszy stan dla dodawania stałej
				if (!stopExe && !stopPC2 && (instr[15:12] == 'b0000)) begin
					stopPC = !stopPC;
					if (!RD) begin
						RD <= 1; WR <= 0;
						DA <= instr[7:4]; DAtempRD <= instr[3:0]; DAtempWR <= instr[11:8]; opCode <= instr[15:12];
					end
				end else if (stopPC) begin
					stopPC = 0;
					if(!stopPC2) stopExe = 1;
				end

				// pierwszy stan instrukcji 1,4-7,8,9
				if (!stopExe && ( (instr[15] == 0 && (instr[14:12] == 1 || instr[14])) || instr[15:12] == 8 || instr [15:12] == 9 )) begin
					if (!RD) begin
						stopPC2 = 1;
						RD <= 1; WR <= 0;
						DA <= instr[7:4]; DAtempRD <= instr[3:0]; DAtempWR <= instr[11:8]; opCode <= instr[15:12];
					end
				end else if(stopPC2) stopExe = 1; 
				
				// drugi stan instrukcji 1,4-7,8,9
				if (RD && stopPC2) begin
					DOUT <= DIN; stopPC = 1;
					if (opCode[3] == 0 && (opCode[2:0] == 1 || opCode[2])) DA <= DAtempRD;
					else if(opCode == 8) begin
						if (DAtempRD[3] == 1) DA <= DIN - (~DAtempRD + 1);
						else DA <= DIN + DAtempRD;
					end else begin
						DA <= DAtempWR;
						if (DAtempRD[3] == 1) DAtempWR <= DIN - (~DAtempRD + 1);
						else DAtempWR <= DIN + DAtempRD;
					end
				end					
					
				if (stopPC || stopPC2) count <= count;
				else begin
					if(instr[15:12] == 'b1111 && !stopExe) begin
						if (instr[11] == 1) count <= count - (~instr + 2);
						else count <= count + instr[11:0] - 1;
					end else count <= count + 1;
				end
		
				led <= instr[15:8];	
			end
end
endmodule

module main(clk, led);
input clk;
output [7:0] led;
wire clk2;
wire [15:0] instr;
wire [2:0] count;

wire [15:0] DOUT;
wire [15:0] DA;
wire [15:0] DIN;
wire WR, RD;

clock clock1 (clk, clk2);
rom rom1(clk, clk2, count, instr);

RAM16X1S ram0 (DIN[0], DA[0], DA[1], DA[2], DA[3], DOUT[0], clk, WR);
RAM16X1S ram1 (DIN[1], DA[0], DA[1], DA[2], DA[3], DOUT[1], clk, WR);
RAM16X1S ram2 (DIN[2], DA[0], DA[1], DA[2], DA[3], DOUT[2], clk, WR);
RAM16X1S ram3 (DIN[3], DA[0], DA[1], DA[2], DA[3], DOUT[3], clk, WR);
RAM16X1S ram4 (DIN[4], DA[0], DA[1], DA[2], DA[3], DOUT[4], clk, WR);
RAM16X1S ram5 (DIN[5], DA[0], DA[1], DA[2], DA[3], DOUT[5], clk, WR);
RAM16X1S ram6 (DIN[6], DA[0], DA[1], DA[2], DA[3], DOUT[6], clk, WR);
RAM16X1S ram7 (DIN[7], DA[0], DA[1], DA[2], DA[3], DOUT[7], clk, WR);
RAM16X1S ram8 (DIN[8], DA[0], DA[1], DA[2], DA[3], DOUT[8], clk, WR);
RAM16X1S ram9 (DIN[9], DA[0], DA[1], DA[2], DA[3], DOUT[9], clk, WR);
RAM16X1S ram10 (DIN[10], DA[0], DA[1], DA[2], DA[3], DOUT[10], clk, WR);
RAM16X1S ram11 (DIN[11], DA[0], DA[1], DA[2], DA[3], DOUT[11], clk, WR);
RAM16X1S ram12 (DIN[12], DA[0], DA[1], DA[2], DA[3], DOUT[12], clk, WR);
RAM16X1S ram13 (DIN[13], DA[0], DA[1], DA[2], DA[3], DOUT[13], clk, WR);
RAM16X1S ram14 (DIN[14], DA[0], DA[1], DA[2], DA[3], DOUT[14], clk, WR);
RAM16X1S ram15 (DIN[15], DA[0], DA[1], DA[2], DA[3], DOUT[15], clk, WR);

cpu cpu1(clk, clk2, count, instr, led, DA, DOUT, DIN, RD, WR);
endmodule

/* testowanie na symulatorze płytki
module test;
reg clk = 0;
wire clk2;
wire [4:0] count;
wire [15:0] instr;
wire [7:0] led;

wire [15:0] DOUT;
wire [15:0] DA;
wire [15:0] DIN;
wire WR, RD;

integer i;

clock clock1 (clk, clk2);
rom rom1(clk, clk2, count, instr);

RAM16X1S ram0 (DIN[0], DA[0], DA[1], DA[2], DA[3], DOUT[0], clk, WR);
RAM16X1S ram1 (DIN[1], DA[0], DA[1], DA[2], DA[3], DOUT[1], clk, WR);
RAM16X1S ram2 (DIN[2], DA[0], DA[1], DA[2], DA[3], DOUT[2], clk, WR);
RAM16X1S ram3 (DIN[3], DA[0], DA[1], DA[2], DA[3], DOUT[3], clk, WR);
RAM16X1S ram4 (DIN[4], DA[0], DA[1], DA[2], DA[3], DOUT[4], clk, WR);
RAM16X1S ram5 (DIN[5], DA[0], DA[1], DA[2], DA[3], DOUT[5], clk, WR);
RAM16X1S ram6 (DIN[6], DA[0], DA[1], DA[2], DA[3], DOUT[6], clk, WR);
RAM16X1S ram7 (DIN[7], DA[0], DA[1], DA[2], DA[3], DOUT[7], clk, WR);
RAM16X1S ram8 (DIN[8], DA[0], DA[1], DA[2], DA[3], DOUT[8], clk, WR);
RAM16X1S ram9 (DIN[9], DA[0], DA[1], DA[2], DA[3], DOUT[9], clk, WR);
RAM16X1S ram10 (DIN[10], DA[0], DA[1], DA[2], DA[3], DOUT[10], clk, WR);
RAM16X1S ram11 (DIN[11], DA[0], DA[1], DA[2], DA[3], DOUT[11], clk, WR);
RAM16X1S ram12 (DIN[12], DA[0], DA[1], DA[2], DA[3], DOUT[12], clk, WR);
RAM16X1S ram13 (DIN[13], DA[0], DA[1], DA[2], DA[3], DOUT[13], clk, WR);
RAM16X1S ram14 (DIN[14], DA[0], DA[1], DA[2], DA[3], DOUT[14], clk, WR);
RAM16X1S ram15 (DIN[15], DA[0], DA[1], DA[2], DA[3], DOUT[15], clk, WR);

cpu cpu1(clk, clk2, count, instr, led, DA, DOUT, DIN, RD, WR);
always @(negedge clk2) $display("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",i, count, led[7:4], led[3:0], instr[15:12], instr[11:8], instr[7:4], instr[3:0], DA, DOUT, DIN, RD, WR);
initial begin
	$display("\t i\tcount\tld74\tld30\tir1512\tir118\tir74\tir30\tDA\tDOUT\tDIN\tRD\tWR");
	for (i=0; i<350; i = i+1) begin
		#1;
		clk =! clk;
	end
end
endmodule
*/