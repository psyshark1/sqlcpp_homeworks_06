//#include "Publisher.h"
#include <iostream>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Windows.h>

#pragma execution_character_set("utf-8")

class Book;
class Stock;
class Sale;

class Publisher
{
public:
    std::string name;
    Wt::Dbo::collection<Wt::Dbo::ptr<Book>> books;
    template<class Action>
    void persist(Action& act)
    {
        Wt::Dbo::field(act, name, "name");
        Wt::Dbo::hasMany(act, books, Wt::Dbo::ManyToOne, "publisher");
    }

private:
};

class Book
{
public:
    std::string title;
    Wt::Dbo::ptr<Publisher> publisher;
    Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;
    template<class Action>
    void persist(Action& act)
    {
        Wt::Dbo::field(act, title, "title");
        Wt::Dbo::belongsTo(act, publisher, "publisher");
        Wt::Dbo::hasMany(act, stocks, Wt::Dbo::ManyToOne, "book");
    }

private:
};

class Shop
{
public:
	std::string name;
	Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;
	template<class Action>
	void persist(Action& act)
    {
        Wt::Dbo::field(act, name, "name");
        Wt::Dbo::hasMany(act, stocks, Wt::Dbo::ManyToOne, "shop");
    }

private:
};

class Stock
{
public:
	int count{0};
	Wt::Dbo::ptr<Book> book;
	Wt::Dbo::ptr<Shop> shop;
	Wt::Dbo::collection<Wt::Dbo::ptr<Sale>> sales;
	template<class Action>
	void persist(Action& act)
    {
        Wt::Dbo::field(act, count, "count");
        Wt::Dbo::belongsTo(act, book, "book");
        Wt::Dbo::belongsTo(act, shop, "shop");
        Wt::Dbo::hasMany(act, sales, Wt::Dbo::ManyToOne, "stock");
    }

private:
};

class Sale
{
public:
	int count{ 0 };
	int price{ 0 };
	std::string date_sale;
	Wt::Dbo::ptr<Stock> stock;
	
	template<class Action>
	void persist(Action& act)
    {
        Wt::Dbo::field(act, count, "count");
        Wt::Dbo::field(act, price, "price");
        Wt::Dbo::field(act, date_sale, "date_sale");
        Wt::Dbo::belongsTo(act, stock, "stock");
    }

private:
};


int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        std::string connectionString{ "dbname=orm_lesson host=127.0.0.1 port=5432 user=postgres password=123" };
        auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(connectionString);

        Wt::Dbo::Session sess;
        sess.setConnection(std::move(conn));
        sess.mapClass<Publisher>("Publisher");
        sess.mapClass<Shop>("Shop");
        sess.mapClass<Book>("Book");
        sess.mapClass<Stock>("Stock");
        sess.mapClass<Sale>("Sale");

        try
        {
            sess.createTables();
        }
        catch (...)
        {

        }
        
        Wt::Dbo::Transaction trns{sess};

        std::unique_ptr<Publisher> publr1{ new Publisher{"AST-Press", {}} };
        std::unique_ptr<Publisher> publr2 { new Publisher{"Drofa", {}} };
        std::unique_ptr<Publisher> publr3 { new Publisher{"Soyuz", {}} };

        auto db_publ1 = sess.add(std::move(publr1));
        auto db_publ2 = sess.add(std::move(publr2));
        auto db_publ3 = sess.add(std::move(publr3));

        std::unique_ptr<Shop> shop1{ new Shop{"Chakona", {}} };
        std::unique_ptr<Shop> shop2{ new Shop{"Bukvoed", {}} };
        std::unique_ptr<Shop> shop3{ new Shop{"Vseznayka", {}} };

        auto Chakona_db = sess.add(std::move(shop1));
        auto Bukvoed_db = sess.add(std::move(shop2));
        auto Vseznayka_db = sess.add(std::move(shop3));

        std::unique_ptr<Book> book1{ new Book{"War and Peace", {}, {}} };
        std::unique_ptr<Book> book2{ new Book{"Foundation", {}, {}} };
        std::unique_ptr<Book> book3{ new Book{"Programming: Principles and Practice Using C++", {}, {}} };

        auto db_book1 = sess.add(std::move(book1));
        auto db_book2 = sess.add(std::move(book2));
        auto db_book3 = sess.add(std::move(book3));

        db_book1.modify()->publisher = db_publ1;
        db_book2.modify()->publisher = db_publ2;
        db_book3.modify()->publisher = db_publ3;

        std::unique_ptr<Stock> stock1{ new Stock{50, {}, {}} };
        std::unique_ptr<Stock> stock2{ new Stock{50, {}, {}} };
        std::unique_ptr<Stock> stock3{ new Stock{50, {}, {}} };

        auto db_stock1 = sess.add(std::move(stock1));
        auto db_stock2 = sess.add(std::move(stock2));
        auto db_stock3 = sess.add(std::move(stock3));

        db_stock1.modify()->book = db_book2;
        db_stock1.modify()->shop = Chakona_db;
        db_stock2.modify()->book = db_book3;
        db_stock2.modify()->shop = Vseznayka_db;
        db_stock3.modify()->book = db_book1;
        db_stock3.modify()->shop = Bukvoed_db;

        std::unique_ptr<Sale> sale1{ new Sale{12, 150, "2023-11-20"} };
        std::unique_ptr<Sale> sale2{ new Sale{31, 380, "2023-11-18"} };
        std::unique_ptr<Sale> sale3{ new Sale{7, 590, "2023-11-21"} };
        std::unique_ptr<Sale> sale4{ new Sale{19, 290, "2023-11-17"} };

        auto db_sale1 = sess.add(std::move(sale1));
        auto db_sale2 = sess.add(std::move(sale2));
        auto db_sale3 = sess.add(std::move(sale3));
        auto db_sale4 = sess.add(std::move(sale4));

        db_sale1.modify()->stock = db_stock2;
        db_sale2.modify()->stock = db_stock1;
        db_sale3.modify()->stock = db_stock3;
        db_sale4.modify()->stock = db_stock2;

        trns.commit();
        std::string query;
        while (true)
        {
            std::cout << "Введите название издателя: ";
            std::cin >> query;
            if (query == "exit") { break; }
            Wt::Dbo::Transaction trns{ sess };
            Wt::Dbo::collection<Wt::Dbo::ptr<Publisher>> pbls = sess.find<Publisher>().where("name = ?").bind(query);
            if (!pbls.empty())
            {
                for (Wt::Dbo::ptr<Publisher>& pbl : pbls)
                {
                    Wt::Dbo::collection<Wt::Dbo::ptr<Book>> bks = sess.find<Book>().where("publisher_id = ?").bind(pbl.id());
                    if (!bks.empty())
                    {
                        std::cout << "Книги издателя " << query << " продаются в:" << std::endl;
                        for (Wt::Dbo::ptr<Book>& bk : bks)
                        {
                            for (Wt::Dbo::ptr<Stock> stk : bk->stocks)
                            {
                                std::cout << stk->shop->name << std::endl;
                            }

                        }
                    }
                    else
                    {
                        std::cout << "Книги издателя " << query << " в продаже отсутствуют" << std::endl;
                    }
                }
            }
            else
            {
                std::cout << "Издатель " << query << " не найден!" << std::endl;
            }
        }
    }
    catch (const std::exception& e)

    {
        std::cout << e.what() << std::endl;
        return 133;
    }
    return 0;
}
