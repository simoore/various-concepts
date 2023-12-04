`timescale 1ns / 1ps

class Generator;

    // rand vs randc
    // rand doesn't depend on previously generated values
    // randc produces all possible combinations before and repititions appear
    //      randc generates a bucket of all combinatiosn. If you change the constraints
    //      a new bucket is created and the uniqueness of combinations no longer holds.
    rand bit [3:0] a, b;
    bit [3:0] y;
    int min;
    int max;
    rand bit wr;

    function void set_range(input int min, input int max);
        this.min = min;
        this.max = max;
    endfunction
    
    // Constrains the random generator
    constraint thisIsTheConstraintName { 
        a inside {[min:max]}; 
        b inside {[min:max]};
    }
    
    // You can give weighted distributions to random signals
    constraint wr_const {
        wr dist { 0 := 10, 1 := 90 };
    }
    
    function void post_randomize();
        $display("The generated value: %0d %0d", a, b);
    endfunction

endclass

module ExamplesRandomization;

    Generator g;
    int i;
    int status;

    initial begin
        g = new();
        for (i = 0; i < 10; i++) begin
            g.set_range(3, 8);
            // Generates values for variables prefixed with rand.
            // Status is true (1) if randomization successful
            status = g.randomize();
            assert (status) else begin
                $display("status assertion failed");
            end
        end
    end

endmodule
