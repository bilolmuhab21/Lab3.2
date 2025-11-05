#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <clocale> 
#ifdef _WIN32
#include <windows.h>
#ifdef max
#undef max
#endif
#endif

using namespace std;

enum class Position {
    Employee = 1,
    Manager,
    Accountant
};

string positionToString(Position p) {
    switch (p) {
    case Position::Employee: return "Сотрудник";
    case Position::Manager: return "Менеджер";
    case Position::Accountant: return "Бухгалтер";
    default: return "Неизвестно";
    }
}

class WorkType {
public:
    int id;
    string name;
    double rate; // оплата за единицу, ставка

    WorkType(int id_, const string& name_, double rate_)
        : id(id_), name(name_), rate(rate_) {}
};


class WorkRecord {
public:
    int workTypeId;
    double quantity; // количество единиц работы (часы, шт. и т.д.)

    WorkRecord(int workTypeId_, double quantity_)
        : workTypeId(workTypeId_), quantity(quantity_) {}
};


class Worker {
private:
    int id;
    string surname;
    Position position;
    vector<WorkRecord> records;

public:
    Worker(int id_, const string& surname_, Position pos_)
        : id(id_), surname(surname_), position(pos_) {}


    int getId() const { return id; }
    string getSurname() const { return surname; }
    Position getPosition() const { return position; }

    void addRecord(const WorkRecord& rec) {
        records.push_back(rec);
    }

    const vector<WorkRecord>& getRecords() const {
        return records;
    }

    // Подсчет зарплаты: принимает список workTypes для определения ставки
    double calculateSalary(const vector<unique_ptr<WorkType>>& workTypes) const {
        double total = 0.0;
        for (const auto& rec : records) {
            auto it = find_if(workTypes.begin(), workTypes.end(),
                [&](const unique_ptr<WorkType>& wt) { return wt->id == rec.workTypeId; });
            if (it != workTypes.end()) {
                total += ((*it)->rate) * rec.quantity;
            }
            // если вид работы не найден — пропускаем (ввод/логика должна предотвращать это)
        }
        return total;
    }

    void printBrief() const {
        cout << "ID: " << id << " | Фамилия: " << surname
            << " | Должность: " << positionToString(position) << "\n";
    }
};

class PayrollSystem {
private:
    PayrollSystem() : nextWorkerId(1), nextWorkTypeId(1) {}

    // данные системы
    vector<unique_ptr<WorkType>> workTypes;
    vector<unique_ptr<Worker>> workers;

    int nextWorkerId;
    int nextWorkTypeId;

public:
    // запрет копирования/перемещения
    PayrollSystem(const PayrollSystem&) = delete;
    PayrollSystem& operator=(const PayrollSystem&) = delete;
    PayrollSystem(PayrollSystem&&) = delete;
    PayrollSystem& operator=(PayrollSystem&&) = delete;

    // получение единственного экземпляра
    static PayrollSystem& getInstance() {
        static PayrollSystem instance;
        return instance;
    }

    // Методы для управления видами работ
    int addWorkType(const string& name, double rate) {
        int id = nextWorkTypeId++;
        workTypes.push_back(make_unique<WorkType>(id, name, rate));
        return id;
    }

    const vector<unique_ptr<WorkType>>& getWorkTypes() const {
        return workTypes;
    }

    const WorkType* findWorkTypeById(int id) const {
        for (const auto& wt : workTypes) {
            if (wt->id == id) return wt.get();
        }
        return nullptr;
    }

    // Методы для управления работниками
    int addWorker(const string& surname, Position pos) {
        int id = nextWorkerId++;
        workers.push_back(make_unique<Worker>(id, surname, pos));
        return id;
    }

    Worker* findWorkerById(int id) {
        for (auto& w : workers) {
            if (w->getId() == id) return w.get();
        }
        return nullptr;
    }

    Worker* findWorkerBySurname(const string& surname) {
        for (auto& w : workers) {
            if (w->getSurname() == surname) return w.get();
        }
        return nullptr;
    }

    const vector<unique_ptr<Worker>>& getWorkers() const {
        return workers;
    }

    // добавить запись о выполненной работе конкретному работнику
    bool addWorkRecordToWorker(int workerId, int workTypeId, double quantity) {
        Worker* w = findWorkerById(workerId);
        if (!w) return false;
        const WorkType* wt = findWorkTypeById(workTypeId);
        if (!wt) return false;
        w->addRecord(WorkRecord(workTypeId, quantity));
        return true;
    }

    // вычислить зарплату для работника по фамилии
    bool getSalaryBySurname(const string& surname, double& outSalary) {
        Worker* w = findWorkerBySurname(surname);
        if (!w) return false;
        outSalary = w->calculateSalary(workTypes);
        return true;
    }

    // общая сумма выплат всем работникам
    double getTotalPayout() const {
        double total = 0.0;
        for (const auto& wptr : workers) {
            total += wptr->calculateSalary(workTypes);
        }
        return total;
    }
};

void clearStdin() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string inputNonEmptyString(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos)
            s = s.substr(start, end - start + 1);
        else
            s = "";

        if (!s.empty()) return s;
        cout << "Ошибка: поле не может быть пустым. Попробуйте снова.\n";
    }
}

double inputPositiveDouble(const string& prompt) {
    double x;
    while (true) {
        cout << prompt;
        if (!(cin >> x)) {
            cout << "Ошибка: введите число.\n";
            cin.clear();
            clearStdin();
            continue;
        }
        clearStdin();
        if (x > 0.0) return x;
        cout << "Ошибка: значение должно быть > 0. Попробуйте снова.\n";
    }
}


int inputPositiveIntLimited(const string& prompt, int maxAllowed = 10000) {
    string line;
    while (true) {
        cout << prompt;
        if (!std::getline(cin, line)) {
            cout << "Ошибка ввода. Попробуйте снова.\n";
            // очистим состояние и попробуем снова
            cin.clear();
            continue;
        }
        // trim
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        if (start == string::npos || end == string::npos) {
            cout << "Ошибка: поле не может быть пустым. Попробуйте снова.\n";
            continue;
        }
        string s = line.substr(start, end - start + 1);

        // Проверка на то, что строка содержит только цифры (без знаков, без десятичного разделителя)
        bool allDigits = !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c); });
        if (!allDigits) {
            cout << "Ошибка: введите целое число.\n";
            continue;
        }

        // Преобразование в число с учётом переполнения
        try {
            long long val = stoll(s);
            if (val <= 0) {
                cout << "Ошибка: значение должно быть > 0. Попробуйте снова.\n";
                continue;
            }
            if (val > maxAllowed) {
                cout << "Ошибка: значение не должно превышать " << maxAllowed << ". Попробуйте снова.\n";
                continue;
            }
            return static_cast<int>(val);
        }
        catch (...) {
            cout << "Ошибка: недопустимое число. Попробуйте снова.\n";
            continue;
        }
    }
}

int inputIntInRange(const string& prompt, int low, int high)
{
    setlocale(LC_ALL, "rus");
    int v;
    while (true) {
        cout << prompt;
        if (!(cin >> v)) {
            cout << "Ошибка: введите целое число.\n";
            cin.clear();
            clearStdin();
            continue;
        }
        clearStdin();
        if (v >= low && v <= high) return v;
        cout << "Ошибка: введите число в диапазоне [" << low << ", " << high << "].\n";
    }
}

int inputMenuChoice(const string& prompt, int low, int high) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            cin.clear();
            continue;
        }
        // trim
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        if (start == string::npos || end == string::npos) {
            cout << "Ошибка: поле не может быть пустым.\n";
            continue;
        }
        string s = line.substr(start, end - start + 1);

        // Проверка: только цифры
        bool allDigits = !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c); });
        if (!allDigits) {
            cout << "Ошибка: введите целое число\n";
            continue;
        }

        int val = stoi(s);
        if (val < low || val > high) {
            cout << "Ошибка: число вне допустимого диапазона [" << low << ", " << high << "].\n";
            continue;
        }
        return val;
    }
}



// выбрать должность
Position choosePosition() {
    cout << "Выберите должность:\n";
    cout << "1. Сотрудник\n2. Менеджер\n3. Бухгалтер\n";
    int choice = inputIntInRange("Ваш выбор (1-3): ", 1, 3);
    return static_cast<Position>(choice);
}

void printMenu() {
    cout << "\n=== Меню отдела расчёта зарплаты ===\n";
    cout << "1. Добавить вид работы\n";
    cout << "2. Показать все виды работ\n";
    cout << "3. Добавить работника\n";
    cout << "4. Показать всех работников\n";
    cout << "5. Добавить выполненную работу работнику\n";
    cout << "6. Вычислить зарплату работника (по фамилии)\n";
    cout << "7. Вывести сумму выплат всем работникам\n";
    cout << "0. Выход\n";
    cout << "===================================\n";
}

void cmdAddWorkType() {
    string name = inputNonEmptyString("Введите название вида работы: ");
    double rate;
    while (true) {
        rate = inputPositiveDouble("Введите ставку (оплату за единицу): ");
        if (rate <= 100000.0) break;
        cout << "Ошибка: ставка не может превышать 100000. Попробуйте снова.\n";
    }
    int id = PayrollSystem::getInstance().addWorkType(name, rate);
    cout << "Вид работы добавлен, ID = " << id << "\n";
}


void cmdListWorkTypes() {
    const auto& wts = PayrollSystem::getInstance().getWorkTypes();
    if (wts.empty()) {
        cout << "Список видов работ пуст.\n";
        return;
    }
    cout << "Список видов работ:\n";
    cout << left << setw(5) << "ID" << setw(25) << "Название" << setw(10) << "Ставка\n";
    for (const auto& wt : wts) {
        cout << left << setw(5) << wt->id
            << setw(25) << wt->name
            << setw(10) << wt->rate << "\n";
    }
}

void cmdAddWorker() {
    string surname = inputNonEmptyString("Введите фамилию работника: ");
    Position pos = choosePosition();
    int id = PayrollSystem::getInstance().addWorker(surname, pos);
    cout << "Работник добавлен. ID = " << id << "\n";
}

void cmdListWorkers() {
    const auto& workers = PayrollSystem::getInstance().getWorkers();
    if (workers.empty()) {
        cout << "Список работников пуст.\n";
        return;
    }
    cout << "Список работников:\n";
    for (const auto& w : workers) {
        w->printBrief();
    }
}

void cmdAddWorkRecord() {
    // Выбрать работника
    cmdListWorkers();
    int workerId = inputIntInRange("Введите ID работника, которому добавляем запись: ", 1, 1000000);
    Worker* w = PayrollSystem::getInstance().findWorkerById(workerId);
    if (!w) {
        cout << "Работник с таким ID не найден.\n";
        return;
    }

    // Выбрать вид работы
    cmdListWorkTypes();
    int workTypeId = inputIntInRange("Введите ID вида работы: ", 1, 1000000);
    const WorkType* wt = PayrollSystem::getInstance().findWorkTypeById(workTypeId);
    if (!wt) {
        cout << "Вид работы с таким ID не найден.\n";
        return;
    }

    int qtyInt = inputPositiveIntLimited("Введите количество выполненных единиц (целое число, не более 10000): ", 10000);
    double qty = static_cast<double>(qtyInt);

    bool ok = PayrollSystem::getInstance().addWorkRecordToWorker(workerId, workTypeId, qty);
    if (ok) {
        cout << "Запись добавлена: работник " << w->getSurname()
            << " выполнил " << qty << " ед. работы '" << wt->name << "'\n";
    }
    else {
        cout << "Ошибка при добавлении записи.\n";
    }
}

void cmdSalaryBySurname() {
    string surname = inputNonEmptyString("Введите фамилию работника для расчёта зарплаты: ");
    double salary;
    if (PayrollSystem::getInstance().getSalaryBySurname(surname, salary)) {
        cout << "Зарплата работника " << surname << " = " << fixed << setprecision(2) << salary << "\n";
    }
    else {
        cout << "Работник с фамилией '" << surname << "' не найден.\n";
    }
}

void cmdTotalPayout() {
    double total = PayrollSystem::getInstance().getTotalPayout();
    cout << "Сумма выплат всем работникам = " << fixed << setprecision(2) << total << "\n";
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    bool running = true;
    while (running) {
        printMenu();
        int choice = inputMenuChoice("Выберите пункт меню: ", 0, 7);
        switch (choice) {
        case 1: cmdAddWorkType(); break;
        case 2: cmdListWorkTypes(); break;
        case 3: cmdAddWorker(); break;
        case 4: cmdListWorkers(); break;
        case 5: cmdAddWorkRecord(); break;
        case 6: cmdSalaryBySurname(); break;
        case 7: cmdTotalPayout(); break;
        case 0:
            running = false;
            cout << "Выход из программы.\n";
            break;
        default:
            cout << "Неверный выбор.\n";
            break;
        }
    }

    cout << "Программа завершена.\n";
    return 0;
}
