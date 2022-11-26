#pragma once
typedef enum {
  E_CMD_OK,
  E_OPEN_FAILED,
  E_CREATE_FAILED,
  E_READ_FAILED,
  E_WRITE_FAILED,
  E_READ_TIME
} types_err;

void putErr(types_err code)
{
  switch(types_err) {
    case E_OPEN_FAILED:
      printf("Failed open to file!\r\n");
    break;

    case E_CREATE_FAILED:
      printf("Failed create to file!\r\n");
    break;

    case E_READ_FAILED:
      printf("Failed read to file!\r\n");
    break;

    case E_WRITE_FAILED:
      printf("Failed write to file!\r\n");
    break;

    case E_READ_TIME:
      printf("Failed read to time!\r\n");
    break;

    default:
      printf("Uncorrect return value command!\r\n");

  }
}