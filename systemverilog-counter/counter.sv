// A minimal design to exercise the arcilator simulation flow: a loadable
// up-counter with an enable and a synchronous reset.
module Counter #(
  parameter int unsigned Width = 8
) (
  input  logic             clock,
  input  logic             reset,
  input  logic             en,
  input  logic             load,
  input  logic [Width-1:0] load_value,
  output logic [Width-1:0] count,
  output logic             overflow
);

  logic [Width-1:0] count_q;

  always_ff @(posedge clock) begin
    if (reset)
      count_q <= '0;
    else if (load)
      count_q <= load_value;
    else if (en)
      count_q <= count_q + 1;
  end

  assign count = count_q;
  assign overflow = en & ~load & ~reset & (count_q == {Width{1'b1}});

endmodule
