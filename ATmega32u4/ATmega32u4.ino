#define half_full_int // from FIFO
#define read_enable // to FIFO
#define output_enable // to tri state buffer
#define bale_int // from Tomcat
#define IOR //from Tomcat
#define address_valid // from decoder/Tomcat
#define IRQ_OUT // to Tomcat

volatile bool bale_flag = false;

void half_full_handler(){
  digitalWrite(IRQ_OUT, HIGH); 
}

void bale_handler(){
  bale_flag = true;
}

void setup() {
  
  pinMode(IRQ_OUT, OUTPUT);
  pinMode(output_enable, OUTPUT);
  pinMode(read_enable, OUTPUT);
  
  attachInterrupt(half_full_int, half_full_handler, RISING); 
  attachInterrupt(bale_int, bale_handler, RISING); 

}

void loop() {
  
  
  if (bale_flag){
    if (address_valid && IOR){
      digitalWrite(read_enable, HIGH);
      digitalWrite(output_enable, HIGH);
    }
    bale_flag = false;
    digitalWrite(read_enable, LOW);
    digitalWrite(output_enable, LOW);
  }


}
