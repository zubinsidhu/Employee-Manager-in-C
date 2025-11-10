/*
 * Homework 5
 * Zubin Sidhu
 * CS240, Spring 2025
 * Purdue University
 */

#include "hw5.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Is Valid Employee - Helper function to check whether employee_t is valid
 * by checking first name and last name, id_number, title, salary and schedule
 */

static int is_valid_employee(employee_t employee) {
  if ((employee.first_name[MAX_NAME_LEN - 1] != '\0') ||
      (strlen(employee.first_name) == MAX_NAME_LEN - 1)) {
    return 0;
  }

  if ((employee.last_name[MAX_NAME_LEN - 1] != '\0') &&
      (strlen(employee.last_name) == MAX_NAME_LEN - 1)) {
    return 0;
  }

  if (employee.id_number == FREE_OFFICE) {
    return 1;
  }
  else {
    if (employee.id_number <= 0) {
      return 0;
    }
    if ((employee.title < TECHNICIAN) || (employee.title > SALESPERSON)) {
      return 0;
    }
    if (employee.salary <= 0.0f) {
      return 0;
    }
    for (int d = 0; d < N_DAYS; d++) {
      for (int h = 0; h < N_HOURS; h++) {
        if ((employee.schedule[d][h] != 'A') &&
            (employee.schedule[d][h] != 'B')) {
          return 0;
        }
      }
    }
    return 1;
  }
} /* is_valid_employee() */

/*
 * Read Employee - Reads the nth employee record from the file. Returns
 * Bad_EMPLOYEE when there is an error
 */

employee_t read_employee(FILE *fp, int n) {
  assert(fp != NULL);
  assert(n >= 0);

  long offset = n * sizeof(employee_t);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  if (offset >= file_size) {
    return BAD_EMPLOYEE;
  }

  if (fseek(fp, offset, SEEK_SET) != 0) {
    return BAD_EMPLOYEE;
  }

  employee_t employee = {0};
  size_t n_read = fread(&employee, sizeof(employee_t), 1, fp);
  if (n_read != 1) {
    return BAD_EMPLOYEE;
  }

  return employee;
} /* read_employee() */

/*
 * Write Employee - Writes the given employee information to the specified file
 * at the nth position. Returns WRITE_ERR if an error occurs
 */

int write_employee(FILE *fp, employee_t employee, int n) {
  assert(fp != NULL);
  assert(n >= 0);
  assert(is_valid_employee(employee));

  long offset = n * sizeof(employee_t);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);

  if (offset > file_size) {
    return WRITE_ERR;
  }

  if (fseek(fp, offset, SEEK_SET) != 0) {
    return WRITE_ERR;
  }

  size_t n_written = fwrite(&employee, sizeof(employee_t), 1, fp);
  if (n_written != 1) {
    return WRITE_ERR;
  }

  return OK;
} /* write_employee() */

/*
 * Hire Employee - Searches for the first FREE_OFFICE in the file and writes
 * the new employee there. Returns the office index on success, NO_OFFICE if
 * none are free, or WRITE_ERR if a writing error occurs
 */

int hire_employee(FILE *fp, employee_t employee) {
  assert(fp != NULL);
  assert(is_valid_employee(employee));

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if (temp.id_number == FREE_OFFICE) {
      int rc = write_employee(fp, employee, i);
      if (rc != OK) {
        return rc;
      }
      return i;
    }
  }

  return NO_OFFICE;
} /* hire_employee() */

/*
 * Fire Employee - Locates the employee in the file whose id, first_name, and
 * last_name match those of the given employee. Sets the employee id_number to
 * FREE_OFFICE. Returns office index of the fired employee on succes or
 * NO_EMPLOYEE if not found
 */

int fire_employee(FILE *fp, employee_t employee) {
  assert(fp != NULL);
  assert(is_valid_employee(employee));

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if ((temp.id_number == employee.id_number) &&
        (strcmp(temp.first_name, employee.first_name) == 0) &&
        (strcmp(temp.last_name, employee.last_name) == 0)) {
      temp.id_number = FREE_OFFICE;
      int rc = write_employee(fp, temp, i);
      if (rc != OK) {
        return rc;
      }
      return i;
    }
  }

  return NO_EMPLOYEE;
} /* fire_employee() */

/*
 * Percent Occupancy - Returns the percent of offices occupied by employees
 * with a salary greater than the value specified by the second parameter
 * of this function
 */

float percent_occupancy(FILE *fp, float min_salary) {
  assert(fp != NULL);
  assert(min_salary >= 0.0f);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  if (total_employees == 0) {
    return 0.0f;
  }

  int count = 0;
  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if ((temp.id_number != FREE_OFFICE) && (temp.id_number > 0)) {
      if (temp.salary > min_salary) {
        count++;
      }
    }
  }

  return 100.0f * ((float) count / (float) total_employees);
} /* percent_occupancy() */

/*
 * Average Salary by Title - Returns the average salary for employees with a
 * given title. If none found, return NO_EMPLOYEE and FREE_OFFICE entries are
 * skipped
 */

float average_salary_by_title(FILE *fp, enum title_t title) {
  assert(fp != NULL);
  assert((title == TECHNICIAN) || (title == MANAGER) || (title == SALESPERSON));

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  float sum = 0.0f;
  int count = 0;
  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if ((temp.id_number != FREE_OFFICE) && (temp.id_number > 0)) {
      if (temp.title == title) {
        sum += temp.salary;
        count++;
      }
    }
  }

  if (count == 0) {
    return NO_EMPLOYEE;
  }
  return sum / count;
} /* average_salary_by_title() */

/*
 * Find Employee by ID - Returns the first employee whose id_number matches.
 * Returns BAD_EMPLOYEE if none found.
 */

employee_t find_employee_by_id(FILE *fp, int id) {
  assert(fp != NULL);
  assert(id > 0);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if (temp.id_number == id) {
      return temp;
    }
  }

  return BAD_EMPLOYEE;
} /* find_employee_by_id() */

/*
 * Find Employee by Name - Returns the first employee whose first and last
 * names match the given arguments. Returns BAD_EMPLOYEE if none found
 */

employee_t find_employee_by_name(FILE *fp, char *first, char *last) {
  assert(fp != NULL);
  assert(first != NULL);
  assert(last != NULL);
  assert(strlen(first) < MAX_NAME_LEN);
  assert(strlen(last) < MAX_NAME_LEN);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if ((strcmp(temp.first_name, first) == 0) &&
        (strcmp(temp.last_name, last) == 0)) {
      return temp;
    }
  }

  return BAD_EMPLOYEE;
} /* find_employee_by_name() */

/*
 * Give Raise - Adds an amount to salary of the first employee whose
 * id number matches the id parameter. If the new salary is less than or
 * equal to 0, set their id number to FREE_OFFICE. Returns OK on successful
 * salary update, NO_EMPLOYEE if no matching id, office number if employee is
 * fired, or WRITE_ERR if writing to the file failed
 */

int give_raise(FILE *fp, int id, float amount) {
  assert(fp != NULL);
  assert(id > 0);

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  int total_employees = (int)(file_size / sizeof(employee_t));

  for (int i = 0; i < total_employees; i++) {
    employee_t temp = read_employee(fp, i);
    if (temp.id_number == id) {
      float new_salary = temp.salary + amount;
      if (new_salary <= 0.0f) {
        temp.id_number = FREE_OFFICE;
        int rc = write_employee(fp, temp, i);
        if (rc != OK) {
          return rc;
        }
        return i;
      }
      else {
        temp.salary = new_salary;
        int rc = write_employee(fp, temp, i);
        if (rc != OK) {
          return rc;
        }
        return OK;
      }
    }
  }

  return NO_EMPLOYEE;
} /* give_raise() */

/*
 * Schedule Meeting - Finds the earliest time in the week both employees are
 * available ('A'). If either employee is not found, return NO_EMPLOYEE  If no
 * common availability, return NO_OVERLAP. Otherwise, return (day * 100 + hour)
 * where day = 0 to 4 (Mon to Fri), and hour = 8 to 16
 */

int schedule_meeting(FILE *fp, int id_one, int id_two) {
  assert(fp != NULL);
  assert(id_one > 0);
  assert(id_two > 0);

  employee_t employee_one = find_employee_by_id(fp, id_one);
  if (employee_one.id_number <= 0) {
    return NO_EMPLOYEE;
  }

  employee_t employee_two = find_employee_by_id(fp, id_two);
  if (employee_two.id_number <= 0) {
    return NO_EMPLOYEE;
  }

  for (int d = 0; d < N_DAYS; d++) {
    for (int h = 0; h < N_HOURS; h++) {
      if ((employee_one.schedule[d][h] == 'A') &&
          (employee_two.schedule[d][h] == 'A')) {
        int hour_of_day = 8 + h;
        return d * 100 + hour_of_day;
      }
    }
  }

  return NO_OVERLAP;
} /* schedule_meeting() */
