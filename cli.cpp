#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "Adafruit_MotorShield.h"

static void usage(const char* prog) {
  fprintf(stderr,
    "Usage:\n"
    "  %s [-b bus] [-a addr] [-f freq] verify\n"
    "  %s [-b bus] [-a addr] [-f freq] dc <motor> <speed%%>\n"
    "  %s [-b bus] [-a addr] [-f freq] stepper <motor> speed <rpm>\n"
    "  %s [-b bus] [-a addr] [-f freq] stepper <motor> step <count> [single|double|interleave|microstep]\n"
    "\n"
    "  motor    : 1-4 for DC, 1-2 for stepper\n"
    "  speed%%   : -100 to 100 (negative = backward, 0 = release)\n"
    "  count    : steps to move (negative = backward)\n"
    "\n"
    "Options:\n"
    "  -b bus   : I2C bus number (default 1)\n"
    "  -a addr  : shield I2C address in hex (default 0x60)\n"
    "  -f freq  : PWM frequency Hz (default 1600)\n",
    prog, prog, prog, prog);
}

int main(int argc, char** argv) {
  int bus = 1;
  uint8_t addr = 0x60;
  uint16_t freq = 1600;

  int i = 1;
  for (; i < argc; i++) {
    if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
      bus = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
      addr = static_cast<uint8_t>(strtol(argv[++i], nullptr, 0));
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      freq = static_cast<uint16_t>(atoi(argv[++i]));
    } else {
      break;
    }
  }

  if (i >= argc) {
    usage(argv[0]);
    return 1;
  }

  Adafruit_MotorShield shield(addr);
  if (!shield.begin(freq, bus)) {
    fprintf(stderr, "error: failed to open I2C bus %d at address 0x%02x\n", bus, addr);
    return 1;
  }

  const char* cmd = argv[i++];

  if (strcmp(cmd, "verify") == 0) {
    if (shield.verify()) {
      printf("ok: PCA9685 responded with expected MODE1 state\n");
      return 0;
    } else {
      fprintf(stderr, "error: PCA9685 did not respond correctly — check wiring and address\n");
      return 1;
    }

  } else if (strcmp(cmd, "dc") == 0) {
    if (i + 1 >= argc) {
      usage(argv[0]);
      return 1;
    }
    int motor = atoi(argv[i++]);
    float pct = atof(argv[i++]);

    if (motor < 1 || motor > 4) {
      fprintf(stderr, "error: dc motor must be 1-4\n");
      return 1;
    }
    if (pct < -100.0f || pct > 100.0f) {
      fprintf(stderr, "error: speed must be -100 to 100\n");
      return 1;
    }

    Adafruit_DCMotor* m = shield.getMotor(motor);
    uint8_t speed = static_cast<uint8_t>((pct < 0 ? -pct : pct) * 255.0f / 100.0f);
    m->setSpeed(speed);
    if (pct > 0)
      m->run(FORWARD);
    else if (pct < 0)
      m->run(BACKWARD);
    else
      m->run(RELEASE);

  } else if (strcmp(cmd, "stepper") == 0) {
    if (i + 1 >= argc) {
      usage(argv[0]);
      return 1;
    }
    int motor = atoi(argv[i++]);
    if (motor < 1 || motor > 2) {
      fprintf(stderr, "error: stepper motor must be 1-2\n");
      return 1;
    }

    const char* subcmd = argv[i++];

    if (strcmp(subcmd, "speed") == 0) {
      if (i >= argc) { usage(argv[0]); return 1; }
      uint16_t rpm = static_cast<uint16_t>(atoi(argv[i++]));
      Adafruit_StepperMotor* s = shield.getStepper(200, motor);
      s->setSpeed(rpm);

    } else if (strcmp(subcmd, "step") == 0) {
      if (i >= argc) { usage(argv[0]); return 1; }
      int count = atoi(argv[i++]);

      uint8_t style = SINGLE;
      if (i < argc) {
        if      (strcmp(argv[i], "double")      == 0) style = DOUBLE;
        else if (strcmp(argv[i], "interleave")  == 0) style = INTERLEAVE;
        else if (strcmp(argv[i], "microstep")   == 0) style = MICROSTEP;
        i++;
      }

      Adafruit_StepperMotor* s = shield.getStepper(200, motor);
      uint8_t dir = count >= 0 ? FORWARD : BACKWARD;
      s->step(static_cast<uint16_t>(count < 0 ? -count : count), dir, style);

    } else {
      fprintf(stderr, "error: unknown stepper subcommand '%s'\n", subcmd);
      usage(argv[0]);
      return 1;
    }

  } else {
    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    usage(argv[0]);
    return 1;
  }

  return 0;
}
