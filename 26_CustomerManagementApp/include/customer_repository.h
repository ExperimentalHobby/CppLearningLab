// CustomerRepository: 顧客管理アプリのデータ層。SQLiteを使うCRUD+検索を
// Win32に一切依存しない形で提供する(21_SQLiteBasicCRUDの TodoRepository と
// 同じ「UI層とデータ層の分離」方針)。
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

struct sqlite3;

namespace customer {

class CustomerRepositoryError : public std::runtime_error {
   public:
    explicit CustomerRepositoryError(const std::string& message) : std::runtime_error(message) {}
};

struct Customer {
    long long id = 0;
    std::string name;
    std::string phone;
    std::string email;
};

class CustomerRepository {
   public:
    CustomerRepository() = default;
    ~CustomerRepository();

    CustomerRepository(const CustomerRepository&) = delete;
    CustomerRepository& operator=(const CustomerRepository&) = delete;

    void Open(const std::string& dbPath);
    void Close();

    long long Add(const Customer& customer);
    std::vector<Customer> List() const;

    // nameQueryを氏名に部分一致(LIKE)で検索する。空文字列なら全件を返す。
    std::vector<Customer> Search(const std::string& nameQuery) const;

    bool Update(const Customer& customer);
    bool Remove(long long id);

   private:
    sqlite3* db_ = nullptr;

    void EnsureSchema();
    std::vector<Customer> RunQuery(const std::string& sql, const std::string& likePattern) const;
};

}  // namespace customer
