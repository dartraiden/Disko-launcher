#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <shlobj.h>

// Функция для перебора всех окон и закрытия окна с определенным классом и заголовком
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	char class_name[80];
	char window_title[80];
	GetClassNameA(hwnd, class_name, sizeof(class_name)); // Получаем имя класса окна
	GetWindowTextA(hwnd, window_title, sizeof(window_title)); // Получаем заголовок окна

	// Если имя класса и заголовок окна соответствуют заданным, то закрываем окно и завершаем программу
	if (strcmp(class_name, "Qt5151QWindowToolSaveBits") == 0 && strcmp(window_title, "Disk-O") == 0) {
		SendMessage(hwnd, WM_CLOSE, 0, 0); // Закрываем окно
		exit(0); // Завершаем программу
	}

	return TRUE; // Продолжаем перебор окон
}

int main() {
	// Получаем путь к папке "Мои документы"
	char my_documents_path[MAX_PATH];
	HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents_path);

	// Создаём путь к файлу "desktop.ini" в папке "Мои документы"
	std::string desktop_ini_path = std::string(my_documents_path) + "\\desktop.ini";

	// Удаляем файл, если он существует
	std::filesystem::remove(desktop_ini_path);

	// Открываем файл для записи в бинарном режиме
	std::ofstream file(desktop_ini_path, std::ios::binary);

	// Записываем BOM для UTF-16 LE
	file.put(0xFF);
	file.put(0xFE);

	// Записываем строки в файл
	std::string str = "\r\n[.ShellClassInfo]\r\nLocalizedResourceName=@%SystemRoot%\\system32\\shell32.dll,-21770\r\nIconResource=%SystemRoot%\\system32\\imageres.dll,-112\r\nIconFile=%SystemRoot%\\system32\\shell32.dll\r\nIconIndex=-235\r\n";
	for (char c : str) {
		file.put(c);  // Записываем младший байт символа
		file.put(0);  // Записываем старший байт символа (0 для ASCII символов)
	}

	file.close(); // Закрываем файл

	// Делаем файл скрытым и системным
	SetFileAttributesA(desktop_ini_path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	HKEY hKey;
	const char* subkey1 = "Software\\Mail.Ru\\Mail.Ru_Disko";
	const char* valueName1 = "licenseLoaded";

	// Открываем ключ реестра
	LONG openRes1 = RegOpenKeyExA(HKEY_CURRENT_USER, subkey1, 0, KEY_ALL_ACCESS, &hKey);

	// Если ключ реестра успешно открыт, то удаляем значение
	if (openRes1 == ERROR_SUCCESS) {
		RegDeleteValueA(hKey, valueName1);
	}

	const char* subkey2 = "Software\\Mail.Ru\\Disk-O\\Keys";

	// Открываем другой ключ реестра
	LONG openRes = RegOpenKeyExA(HKEY_CURRENT_USER, subkey2, 0, KEY_ALL_ACCESS, &hKey);

	// Если ключ реестра успешно открыт, то удаляем все подключи, начинающиеся с "License"
	if (openRes == ERROR_SUCCESS) {
		DWORD i = 0;
		char lpSubKey[256];
		DWORD cch = sizeof(lpSubKey);
		std::vector<std::string> keysToDelete;

		// Перебираем все подключи
		while (RegEnumKeyExA(hKey, i++, lpSubKey, &cch, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			// Если подключ начинается с "License", то добавляем его в список для удаления
			if (strncmp(lpSubKey, "License", 7) == 0) {
				keysToDelete.push_back(lpSubKey);
			}
			cch = sizeof(lpSubKey);
		}

		// Удаляем все подключи из списка
		for (const auto& key : keysToDelete) {
			RegDeleteKeyA(hKey, key.c_str());
		}
	}

	// Если файл "Disko.exe" существует, то запускаем его с параметром "-disableUpdate"
	if (std::filesystem::exists(".\\vcurrent\\Disko.exe")) {
		system("start .\\vcurrent\\Disko.exe -disableUpdate");
	}
	else {
		// Если файл "Disko.exe" не найден, то выводим сообщение об ошибке и завершаем работу
		MessageBoxA(NULL, "No Disk-O executable was found. Place launcher to %LocalAppData%\\Mail.Ru\\Disk-O", "Error", MB_ICONERROR | MB_OK);
		return 1;
	}

	// В бесконечном цикле каждые 5 секунд перебираем все окна и закрываем окно с определенным классом и заголовком
	while (true) {
		Sleep(5000);
		EnumWindows(EnumWindowsProc, 0);
	}

	return 0;
}