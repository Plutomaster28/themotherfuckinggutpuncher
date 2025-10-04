# GutPuncher Makefile
# Builds static executable with UCRT64

CC = gcc
TARGET = gutpuncher.exe
SRC = gutpuncher.c

# Compiler flags for static linking with UCRT64
CFLAGS = -O2 -Wall -Wextra -municode -D_UNICODE -DUNICODE
LDFLAGS = -static -static-libgcc -lpsapi -lshlwapi -ladvapi32 -lkernel32 -luser32

# Release build (static, optimized)
all: $(TARGET)

$(TARGET): $(SRC)
	@echo Building GutPuncher (static executable)...
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)
	@echo Build complete: $(TARGET)
	@echo File size: 
	@powershell -Command "(Get-Item $(TARGET)).Length / 1KB" | findstr /R "^" && echo KB

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Clean build artifacts
clean:
	@echo Cleaning build artifacts...
	@if exist $(TARGET) del $(TARGET)
	@if exist *.o del *.o
	@echo Clean complete.

# Install to system (requires admin)
install: $(TARGET)
	@echo Installing to C:\Windows\System32...
	@copy $(TARGET) C:\Windows\System32\$(TARGET)
	@echo Installation complete.

# Strip symbols for smaller size
strip: $(TARGET)
	@echo Stripping symbols...
	strip -s $(TARGET)
	@echo Strip complete.

# Create release package
release: clean all strip
	@echo Creating release package...
	@if not exist release mkdir release
	@copy $(TARGET) release\
	@copy README.md release\ 2>nul || echo No README found
	@echo Release package created in release\

.PHONY: all debug clean install strip release
