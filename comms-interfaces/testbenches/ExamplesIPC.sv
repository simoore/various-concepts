`timescale 1ns / 1ps

module ExamplesIPC;

    ///////////////////////////////////////////////////////////////////////////
    // This shows the use of an event to transfer information between to 
    // verilog processes.
    ///////////////////////////////////////////////////////////////////////////
    
    event a;

    initial begin
        #10;
        -> a;
        #10;
        -> a;
    end
    
    initial begin
        wait(a.triggered);  // level sensitive
        $display("received event 1 at %0t", $time);
        @(a);               // edge sensitive
        $display("received event 2 at %0t", $time);
    end
    
    ///////////////////////////////////////////////////////////////////////////
    // Use fork/join to launch parallel tasks and events can be used for
    // synchronization.
    //
    // There are three kinds of joins
    //  join        waits for all tasks to finish
    //  join_any    waits for at least one task to finish
    //  join_none   joesn't wait for any task to finish
    ///////////////////////////////////////////////////////////////////////////

    int i = 0;
    bit [7:0] data1;
    bit [7:0] data2;
    event done;
    event next;

    task generator();   
        for (i = 0; i < 10; i++) begin  
            data1 = $urandom();
            $display("Data Sent : %0d", data1);
            #10;
            wait(next.triggered);
        end
        -> done; 
    endtask
      
    task receiver();
        forever begin
            #10;
            data2 = data1;
            $display("Data RCVD : %0d", data2);
            ->next; 
        end 
    endtask
  
    task wait_event();
        wait(done.triggered);
        $display("Completed Sending all Stimulus");
        $finish();
    endtask
  
    initial begin
        fork
            generator();
            receiver();
            wait_event();
        join 
    end

endmodule
