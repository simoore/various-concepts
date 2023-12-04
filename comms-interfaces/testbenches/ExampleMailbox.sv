`timescale 1ns / 1ps

module ExampleMailbox;

    class Transaction;
        bit [7:0] data;
    endclass
     
    class Generator;
        int data = 12;
        Transaction t;
        mailbox #(Transaction) mbx;
        logic [7:0] temp = 3;
        
        function new(mailbox #(Transaction) mbx);
            this.mbx = mbx;
        endfunction
    
        task run();
            t = new();
            t.data = 45;
            mbx.put(t);
            $display("[GEN] : Data Send from Gen : %0d ",t.data);
        endtask
    
    endclass

    class Driver;
        mailbox #(Transaction) mbx;
        Transaction data;
        
        function new(mailbox #(Transaction) mbx);
            this.mbx = mbx;
        endfunction

        task run();
            mbx.get(data);
            $display("[DRV] : DATA rcvd : %0d",data.data);
        endtask
    endclass

    Generator gen;
    Driver drv;
    mailbox #(Transaction) mbx;

    initial begin
        mbx = new();
        gen = new(mbx);
        drv = new(mbx); 
        
        gen.run();
        drv.run();
    end

endmodule
