import os
import sys
import json
import subprocess
from pathlib import Path
from datetime import datetime

class BSDOSManager:
    def __init__(self):
        self.errors_file = Path("errors.json")
        self.load_errors()
        
    def load_errors(self):
        if self.errors_file.exists():
            with open(self.errors_file, 'r') as f:
                self.errors = json.load(f)
        else:
            self.errors = {}
    
    def save_errors(self):
        with open(self.errors_file, 'w') as f:
            json.dump(self.errors, f, indent=2)
    
    def new_ercode(self, code):
        if code in self.errors:
            return f"❌ Код {code} уже существует"
        self.errors[code] = {
            "name": None,
            "created": datetime.now().isoformat(),
            "count": 0
        }
        self.save_errors()
        return f"✅ Создан код ошибки: {code}"
    
    def new_ername(self, code, name):
        if code not in self.errors:
            return f"❌ Код {code} не найден"
        self.errors[code]["name"] = name.upper()
        self.save_errors()
        return f"✅ Добавлено имя {name.upper()} к {code}"
    
    def call_bsod(self, code, admin=True):
        if code not in self.errors:
            return f"❌ Код {code} не найден"
        
        if admin:
            print("🔴 ТРЕБУЮТСЯ ПРАВА АДМИНИСТРАТОРА!")
            return
        
        # Проверяем наличие драйвера
        driver_path = Path("bsod_driver.sys")
        if not driver_path.exists():
            print("⚠️ Драйвер не найден! Загружаем...")
            result = subprocess.run(["sc", "start", "BSODDriver"], 
                                  capture_output=True, text=True)
            if result.returncode != 0:
                print("❌ Не удалось загрузить драйвер!")
                print(result.stderr)
                return
        
        # Запускаем trigger
        error_code = self.errors[code]
        trigger_path = Path("bsod_trigger.exe")
        if trigger_path.exists():
            print(f"💀 Вызов BSOD с кодом: {code}")
            subprocess.run([str(trigger_path), code])
        else:
            print("❌ bsod_trigger.exe не найден!")
    
    def list_errors(self):
        if not self.errors:
            print("Нет сохранённых ошибок")
            return
        print("\nСохранённые ошибки:")
        print("-" * 50)
        for code, data in self.errors.items():
            name = data.get('name', 'Без имени')
            created = data.get('created', 'Неизвестно')
            print(f"🔹 {code} - {name} (создана: {created})")

def main():
    if len(sys.argv) < 2:
        print("""
╔═══════════════════════════════════════════════╗
║          🟦 BSDOS Tool v2.0 🟦             ║
╠═══════════════════════════════════════════════╣
║ Команды:                                     ║
║ bsdos new ercode <код>                      ║
║ bsdos new ername <код> <имя>               ║
║ bsdos call <код>                           ║
║ bsdos list                                  ║
╚═══════════════════════════════════════════════╝
        """)
        return
    
    manager = BSDOSManager()
    cmd = sys.argv[1]
    
    if cmd == "new":
        if len(sys.argv) < 4:
            print("❌ Недостаточно аргументов")
            return
        
        if sys.argv[2] == "ercode":
            print(manager.new_ercode(sys.argv[3]))
        elif sys.argv[2] == "ername":
            if len(sys.argv) < 5:
                print("❌ Не указано имя ошибки")
                return
            print(manager.new_ername(sys.argv[3], sys.argv[4]))
        else:
            print("❌ Неизвестная подкоманда")
    
    elif cmd == "call":
        if len(sys.argv) < 3:
            print("❌ Не указан код ошибки")
            return
        # Проверяем, есть ли non_admin
        is_admin = not (len(sys.argv) > 3 and sys.argv[3] == "non_admin")
        manager.call_bsod(sys.argv[2], is_admin)
    
    elif cmd == "list":
        manager.list_errors()
    
    else:
        print(f"❌ Неизвестная команда: {cmd}")

if __name__ == "__main__":
    main()