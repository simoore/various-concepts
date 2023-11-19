module SpiMaster #(
    parameter DW = 12
) (
    input logic clk, 
    input logic newd,
    input logic rst,
    input logic [DW-1:0] din, 
    output logic sclk,
    output logic cs,
    output logic mosi
);

    typedef enum bit [1:0] {
        Idle = 2'b00, 
        Enable = 2'b01, 
        Send = 2'b10, 
        Comp = 2'b11 
    } State;

    State state = Idle;
    int countc = 0;
    int count = 0;

    /////////////////////////generation of sclk

    always@(posedge clk) begin
        if(rst == 1'b1) begin
            countc <= 0;
            sclk <= 1'b0;
        end else begin 
            if (countc < 10) begin /// fclk / 20
                countc <= countc + 1;
            end else begin
                countc <= 0;
                sclk <= ~sclk;
            end
        end
    end

    //////////////////state machine
    reg [11:0] temp;

    always@(posedge sclk) begin
        if(rst == 1'b1) begin
            cs <= 1'b1; 
            mosi <= 1'b0;
        end else begin
            case(state)
            Idle: begin
                if(newd == 1'b1) begin
                    state <= Send;
                    temp <= din; 
                    cs <= 1'b0;
                end else begin
                    state <= Idle;
                    temp <= 8'h00;
                end
            end


            Send : begin
                if(count <= DW-1) begin
                    mosi <= temp[count]; /////sending lsb first
                    count <= count + 1;
                end else begin
                    count <= 0;
                    state <= Idle;
                    cs <= 1'b1;
                    mosi <= 1'b0;
                end
            end

            default: 
                state <= Idle; 

            endcase
        end 
    end

endmodule : SpiMaster
