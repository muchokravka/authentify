# Daily Identity Code Generator (C++)

Simple CLI application that generates **three deterministic daily codes** from:

- Day
- Month
- Year
- One shared secret password

It does **not** use time-of-day, so the same date + password always produce identical codes across machines.

## Features

- Deterministic code generation (date + password only)
- Generates 3 separate codes per day:
  - User #1 verification code
  - User #2 verification code
  - File encryption code
- Input validation for date and empty password
- Copy-friendly one-line output for quick clipboard usage

## File Structure

- `daily_identity_codes.cpp` — main CLI application source

## Build

From the project root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic daily_identity_codes.cpp -o daily_identity_codes
```

## Run

```bash
./daily_identity_codes
```

You will be prompted for:

1. Day
2. Month
3. Year
4. Shared secret password

## Example

Example interactive session:

```text
===============================================
        DAILY IDENTITY CODE GENERATOR
===============================================
Generate matching daily verification codes
for two users + one file encryption code.
(Date + shared password, no time used)

Enter day   (1-31): 23
Enter month (1-12): 4
Enter year  (e.g. 2026): 2026

Enter shared secret password: MySharedPass123!

--------------- GENERATED CODES ---------------
Date: 23-04-2026

User #1 Verification Code : 2XL9-NQQJ-BEDR-22Y9
User #2 Verification Code : YNDG-EVB8-O2CP-3E0G
File Encryption Code      : 3T0D-FZNP-CSSU-DLP5
-----------------------------------------------

Copy-all line (easy clipboard use):
23-04-2026|2XL9-NQQJ-BEDR-22Y9|YNDG-EVB8-O2CP-3E0G|3T0D-FZNP-CSSU-DLP5
```

## How It Works (High Level)

For each code role (`USER1`, `USER2`, `FILE_ENCRYPT`), the app combines role + date + password into a source string, hashes it deterministically, then formats the result into grouped alphanumeric code blocks.

This guarantees:

- Same date + same password => same 3 codes
- Different date or password => different codes
- Each role gets its own unique code for the same day

## Notes

- This project is intended for simple identity-code workflows in CLI environments.
- Keep the shared secret password private.
