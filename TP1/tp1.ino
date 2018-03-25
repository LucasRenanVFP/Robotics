#include <LiquidCrystal.h>
#include <Adafruit_MotorShield.h>

int x_speed;
int y_speed;
int x_max_speed;
int y_max_speed;
int x_traverse_time;
int y_traverse_time;
int x_length_per_letter;
int x_length_per_space;
bool running;
bool speeds_set;
bool times_set;
bool first_run;

const int button_pin = 0;
const int x_speed_step = 5;
const int y_speed_step = 5;
const int x_time_step = 20;
const int y_time_step = 20;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *motor_x = AFMS.getMotor(1);
Adafruit_DCMotor *motor_y = AFMS.getMotor(2);

void setup() {
  y_speed = x_speed = 0;
  y_max_speed = x_max_speed = 0;
  y_traverse_time = x_traverse_time = 0;
  running = speeds_set = times_set = false;
  first_run = true;
  lcd.begin(16, 2);
  AFMS.begin();
}

bool DOWN(int button) { return button < 50;}

bool UP(int button) { return button < 100;}

bool SELECT(int button) { return button < 400;}

bool LEFT(int button) { return button < 600;}

bool RIGHT(int button) { return button < 800;}

void printSpeeds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("x-speed: " + String(x_max_speed));
  lcd.setCursor(0, 1);
  lcd.print("y-speed: " + String(y_max_speed));
}

void printTimes() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("x-time: " + String(x_traverse_time));
  lcd.setCursor(0, 1);
  lcd.print("y-time: " + String(y_traverse_time));
}

void printRun() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start writing?");
}

void menu() {
  lcd.clear();
  int button = analogRead(button_pin);
  if(first_run) { 
    first_run = false;
    printSpeeds();
  }
  if(!speeds_set) {
    bool button_pressed = false;
    if(DOWN(button)) {
      y_max_speed -= y_speed_step;
      button_pressed = true;
    }
    else if(UP(button)) {
      y_max_speed += y_speed_step;
      button_pressed = true;
    }
    else if(LEFT(button)) {
      x_max_speed -= x_speed_step;
      button_pressed = true;
    }
    else if(RIGHT(button)) {
      x_max_speed += x_speed_step;
      button_pressed = true;
    }
    else if(SELECT(button)) {
      speeds_set = true;
      button_pressed = true;
    }
    x_max_speed = min(x_max_speed, 255);
    x_max_speed = max(x_max_speed, -255);
    y_max_speed = min(y_max_speed, 255);
    y_max_speed = max(y_max_speed, -255);
    if(!speeds_set) printSpeeds();
    else printTimes();
    if(button_pressed) {
      delay(250);
    }
  }
  else if(!times_set){
    bool button_pressed = false;
    if(DOWN(button)) {
      y_traverse_time -= y_time_step;
      button_pressed = true;
    }
    else if(UP(button)) {
      y_traverse_time += y_time_step;
      button_pressed = true;
    }
    else if(LEFT(button)) {
      x_traverse_time -= x_time_step;
      button_pressed = true;
    }
    else if(RIGHT(button)) {
      x_traverse_time += x_time_step;
      button_pressed = true;
    }
    else if(SELECT(button)) {
      times_set = true;
      button_pressed = true;
    }
    if(!times_set) printTimes();
    else printRun();
    if(button_pressed) {
      delay(250);
    }
  }
  else {
    printRun();
    if(SELECT(button)) {
      running = true;
    }
  }
}

void run_x(int speed) {
  motor_x->setSpeed(abs(speed));
  if(speed > 0) {
    motor_x->run(FORWARD);
  }
  else {
    motor_x->run(BACKWARD);
  }
}

void run_y(int speed) {
  motor_y->setSpeed(abs(speed));
  if(speed > 0) {
    motor_y->run(FORWARD);
  }
  else {
    motor_y->run(BACKWARD);
  }
}

void stop_x() {
  motor_x->run(RELEASE);
}

void stop_y() {
  motor_y->run(RELEASE);
}

void writeSpace() {
  run_x(x_max_speed);
  delay(x_length_per_space);
  stop_x();
}

void verticalLine(int direction, int time = y_traverse_time) {
  int speed = y_max_speed * direction;
  run_y(speed);
  delay(time);
  stop_y();
}

void horizontalLine(int direction, int time = x_length_per_letter) {
  int speed = x_max_speed * direction;
  run_x(speed);
  delay(time);
  stop_x();
}

void writeSpace() {
  horizontalLine(1, x_length_per_space);  
}

void writeU() {
  //Assuming it starts at the upper and left extremes of the robot workplace
  verticalLine(-1);
  horizontalLine(1);
  verticalLine(1);
}

void writeF() {
  horizontalLine(1);
  horizontalLine(-1);
  verticalLine(-1, y_traverse_time / 3);
  horizontalLine(1);
  horizontalLine(-1);
  verticalLine(-1, y_traverse_time - (y_traverse_time / 3));
  verticalLine(1);
  horizontalLine(1);
}

void diagonalLine(int direction, int time = x_length_per_letter / 2) {
  /*
    Possibly, it'll be needed to implement some kind of multiplier to some of the speed to
    prevent something like |\___/| to happen or even |`'|
  */
  run_x(x_max_speed);
  run_y(y_max_speed * direction);
  delay(time);
  stop_x();
  stop_y();
}

void writeM() {
  verticalLine(-1);
  verticalLine(1);
  diagonalLine(1);
  diagonalLine(-1, x_length_per_letter - (x_length_per_letter / 2));
  verticalLine(-1);
  verticalLine(1);
}

void writeG() {
  horizontalLine(1);
  horizontalLine(-1);
  verticalLine(-1);
  horizontalLine(1);
  verticalLine(1, y_traverse_time / 2);
  horizontalLine(-1, x_length_per_letter / 2);
  verticalLine(-1, y_traverse_time / 3);
}
  


void run() {
  /*
    Robot should write U(space)F(space)M(space)G. Considering space should be 1/5 of each letter length,
    we have x_traverse_time / 23 per space and 5 times this per letter;
  */
  x_length_per_space = x_traverse_time / 23;
  x_length_per_letter = x_length_per_space * 5;
  writeU();
  writeSpace();
  writeF();
  writeSpace();
  writeM();
  writeSpace();
  writeG();
  running = false;
}

void loop() {
  if(!running) {
    menu();
  }
  else { 
    run();
  }
}
