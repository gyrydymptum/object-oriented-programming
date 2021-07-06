
#define CATCH_CONFIG_MAIN

#include <catch/catch.hpp>

#include "Alcohol.hpp"
#include "Fruit.hpp"
#include "Item.hpp"
#include "Store.hpp"

constexpr auto moreThanFullQuantity = 101;
constexpr auto fullQuantity = 100;
constexpr auto halfQuantity = 50;
//constexpr auto basePrice = 10;
constexpr auto cargoTypes = 7;

constexpr auto fullCappacity = cargoTypes;
constexpr auto lessCappacity = 6;

constexpr auto noExistingCargoName = "Dyes";
constexpr auto existingCargoName = "Wheat";

constexpr auto singleCargoCost = 10;
constexpr auto playerMoney = singleCargoCost * fullQuantity * cargoTypes;
constexpr auto playerMoneyForHalfQuantity = singleCargoCost * halfQuantity;

constexpr auto fruitBestBefore = 5u;
constexpr auto alcoholPercentage = 40u;
constexpr auto itemQuality = Quality::rare;

constexpr auto cargoDefaultIndex = 0;
constexpr auto secondCargoDefaultIndex = 1;
constexpr auto fruitIndex = 4;
constexpr auto alcoholIndex = 5;
constexpr auto itemIndex = 6;

constexpr auto defaultGoodsName = "Towar";

// Return by value to give up ownership
std::unique_ptr<Cargo> generateCargo(size_t index, const std::string& differentName = std::string{}) {
    if (index >= cargoTypes) {
        throw std::out_of_range("generateCargo: index is out of bounds.");
    }

    // https://portroyale3.fandom.com/wiki/Goods
    const std::vector<std::pair<std::string, size_t>> goods{
        {"Wood", singleCargoCost},
        {"Adobe Bricks", singleCargoCost},
        {"Wheat", singleCargoCost},
        {"Hemp", singleCargoCost},
        {"Fruits", singleCargoCost},      //4 - fruit
        {"Corn thing", singleCargoCost},  //5 - alcohol
        {"Box", singleCargoCost}          //6 - item
    };

    std::string goodsName;
    if (differentName.empty()) {
        goodsName = goods[index].first;
    } else {
        goodsName = differentName;
    }

    switch (index) {
    case fruitIndex:
        return std::make_unique<Fruit>(goodsName, fullQuantity, goods[index].second, fruitBestBefore);
    case alcoholIndex:
        Alcohol::alcoholBasePriceFor96percent_ = singleCargoCost;
        return std::make_unique<Alcohol>(goodsName, fullQuantity, alcoholPercentage);
    case itemIndex:
        return std::make_unique<Item>(goodsName, fullQuantity, goods[index].second, itemQuality);
    default:
        return std::make_unique<CargoDefault>(goodsName, fullQuantity, goods[index].second);
    }
}

//return pointer to existing cargo
Cargo* getStoreCargo(const Store& store, const std::string& cargoName, size_t amount) {
    CargoDefault exemplar{cargoName, amount, singleCargoCost};
    auto cargoPtr = store.findCargoInStore(&exemplar);
    if (cargoPtr) {
        std::cout << "Found: " << cargoPtr->getName() << '\n';
        return cargoPtr;
    }
    std::cout << "Not Found: " << cargoName << '\n';
    return nullptr;
}

void fillStore(Store& store) {
    for (int i = 0; i < cargoTypes; ++i) {
        auto cargo = generateCargo(i);
        store.load(std::move(cargo));
    }
}

void fillShip(Ship* ship) {
    if (!ship) {
        return;
    }

    for (int i = 0; i < cargoTypes; ++i) {
        auto cargo = generateCargo(i);
        ship->load(std::move(cargo));
    }
}

//Fruit::operator-- test

TEST_CASE("Fruit decrement price over time", "[Fruit]") {
    auto fruit = generateCargo(fruitIndex);

    REQUIRE(fruit->getPrice() == singleCargoCost);
    auto& fruitTypeRef = dynamic_cast<Fruit&>(*fruit);
    for (int i = 1; i <= fruitBestBefore; ++i) {
        --fruitTypeRef;
        REQUIRE(fruit->getPrice() == singleCargoCost * (fruitBestBefore - i) / fruitBestBefore);
    }
}

//Cargo types operator== tests

TEST_CASE("Compare same CargoDefault", "[Alcohol][Cargo]") {
    auto cargoDefault = generateCargo(cargoDefaultIndex);
    auto samecargoDefault = generateCargo(cargoDefaultIndex);

    REQUIRE(*samecargoDefault == *cargoDefault);
}

TEST_CASE("Compare CargoDefault to CargoDefault with same name", "[Alcohol][Cargo]") {
    auto cargoDefault = generateCargo(cargoDefaultIndex, defaultGoodsName);
    auto secondcargoDefault = generateCargo(secondCargoDefaultIndex, defaultGoodsName);

    REQUIRE(*secondcargoDefault == *cargoDefault);  // Unimportant because CargoDefault is temporary
}

TEST_CASE("Compare different CargoDefault", "[Alcohol][Cargo]") {
    auto cargoDefault = generateCargo(cargoDefaultIndex);
    auto secondcargoDefault = generateCargo(secondCargoDefaultIndex);

    REQUIRE(!(*secondcargoDefault == *cargoDefault));
}

TEST_CASE("Compare Alcohol to CargoDefault with same name", "[Alcohol][Cargo]") {
    auto alcohol = generateCargo(alcoholIndex, defaultGoodsName);
    auto cargoDefault = generateCargo(cargoDefaultIndex, defaultGoodsName);

    REQUIRE(!(*alcohol == *cargoDefault));
}

/*TEST_CASE("Compare CargoDefault to Alcohol with same name", "[Alcohol][Cargo]") {
    auto alcohol = generateCargo(alcoholIndex, defaultGoodsName);
    auto cargoDefault = generateCargo(cargoDefaultIndex, defaultGoodsName);

    CHECK(!(*cargoDefault == *alcohol));  // Unimportant because CargoDefault is temporary
}*/

TEST_CASE("Compare Fruit to Alcohol with same name", "[Alcohol][Fruit][Cargo]") {
    auto alcohol = generateCargo(alcoholIndex, defaultGoodsName);
    auto fruit = generateCargo(fruitIndex, defaultGoodsName);

    REQUIRE(!(*fruit == *alcohol));
}

//Store::findCargoInStore tests

TEST_CASE("findCargoInStore by nullptr", "[Store]") {
    Store store;
    fillStore(store);
    auto noExisting = store.findCargoInStore(nullptr);
    REQUIRE(!noExisting);
}

TEST_CASE("findCargoInStore by noExistingCargoName", "[Store]") {
    Store store;
    fillStore(store);
    auto noExisting = getStoreCargo(store, noExistingCargoName, fullQuantity);
    REQUIRE(!noExisting);
}

TEST_CASE("findCargoInStore by existingCargoName", "[Store]") {
    Store store;
    fillStore(store);
    auto existing = getStoreCargo(store, existingCargoName, fullQuantity);
    REQUIRE(existing);
}

TEST_CASE("findCargoInStore by exemplar Cargo no meter of quantity to search", "[Store]") {
    Store store;
    fillStore(store);

    auto existing1 = getStoreCargo(store, existingCargoName, moreThanFullQuantity);
    auto existing2 = getStoreCargo(store, existingCargoName, fullQuantity);
    auto existing3 = getStoreCargo(store, existingCargoName, halfQuantity);
    auto existing4 = getStoreCargo(store, existingCargoName, 0);

    REQUIRE(existing1);
    REQUIRE(existing2);
    REQUIRE(existing3);
    REQUIRE(existing4);
}

TEST_CASE("findCargoInStore will ignore own Cargo ptr", "[Store]") {
    Store store;
    fillStore(store);

    auto existing = getStoreCargo(store, existingCargoName, fullQuantity);
    auto selfCargo = store.findCargoInStore(existing);

    REQUIRE(!selfCargo);
}

//Store::unload tests

TEST_CASE("unload will ignore nullptr", "[Store]") {
    Store store;
    fillStore(store);

    REQUIRE_NOTHROW(store.unload(nullptr));
}

TEST_CASE("unload will throw on wrong ptr", "[Store]") {
    Store store;
    fillStore(store);

    CargoDefault exemplar{noExistingCargoName, fullQuantity, singleCargoCost};

    REQUIRE_THROWS(store.unload(&exemplar));
    REQUIRE_THROWS_AS(store.unload(&exemplar), std::logic_error);
    REQUIRE_THROWS_WITH(store.unload(&exemplar), "Store: Not my Cargo!");
}

TEST_CASE("unload will remove cargo", "[Store]") {
    Store store;
    fillStore(store);

    auto num_of_cargo_before = store.getCargoNum();

    auto existing = getStoreCargo(store, existingCargoName, fullQuantity);
    store.unload(existing);
    auto noExistNow = getStoreCargo(store, existingCargoName, fullQuantity);

    auto num_of_cargo_after = store.getCargoNum();

    REQUIRE(!noExistNow);
    REQUIRE(num_of_cargo_before == num_of_cargo_after + 1);
}

//Store::load tests

TEST_CASE("load will ignore nullptr", "[Store]") {
    Store store;
    auto emptyCargo = std::unique_ptr<Cargo>();

    store.load(std::move(emptyCargo));

    REQUIRE(store.getCargoNum() == 0);
}

TEST_CASE("load will insert cargo", "[Store]") {
    Store store;
    fillStore(store);

    REQUIRE(store.getCargoNum() == cargoTypes);
}

TEST_CASE("load will increase amount of same cargo", "[Store]") {
    Store store;
    fillStore(store);

    auto cargo = generateCargo(0);
    store.load(std::move(cargo));

    cargo = generateCargo(0);
    Cargo* cargoIn = store.findCargoInStore(cargo.get());
    REQUIRE(cargoIn->getAmount() == (2 * fullQuantity));
}

//Store::buy tests

TEST_CASE("no player to sell to empty store", "[Store][buy]") {
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), fullQuantity, nullptr);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("player sell without ship to empty store", "[Player][Store][buy]") {
    auto ship = std::unique_ptr<Ship>{};
    Player player(std::move(ship), playerMoney);
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("player sell noexisting cargo to empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("player sell more single cargo than have to empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    fillShip(player.getShip());
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), moreThanFullQuantity, &player);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("player sell half single cargo than have to empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    fillShip(player.getShip());
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), halfQuantity, &player);

    REQUIRE(response == Response::done);
    REQUIRE(player.getAvailableSpace() == 0);
    REQUIRE(store.getCargoNum() == 1);
}

TEST_CASE("player sell full single cargo to empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    fillShip(player.getShip());
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.buy(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::done);
    REQUIRE(player.getAvailableSpace() == 1);
    REQUIRE(store.getCargoNum() == 1);
}

TEST_CASE("player sell full of all cargo to empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    fillShip(player.getShip());

    REQUIRE(player.getAvailableSpace() == 0);

    Store store;

    for (int i = 0; i < cargoTypes; ++i) {
        auto cargoExample = generateCargo(i);
        auto response = store.buy(cargoExample.get(), fullQuantity, &player);
        REQUIRE(response == Response::done);
    }

    REQUIRE(player.getAvailableSpace() == fullCappacity);
    REQUIRE(store.getCargoNum() == fullCappacity);
}

//Store::sell tests

TEST_CASE("no player to buy from empty store", "[Store][sell]") {
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), fullQuantity, nullptr);

    REQUIRE(response == Response::lack_of_space);
}

TEST_CASE("rich player buy without ship from empty store", "[Player][Store][sell]") {
    auto ship = std::unique_ptr<Ship>{};
    Player player(std::move(ship), playerMoney);
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::lack_of_space);
}

TEST_CASE("rich player with empty ship buy noexisting cargo from empty store", "[Player][Ship][Store][sell]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    Store store;

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("rich player with empty ship buy more cargo than have store have", "[Player][Ship][Store][sell]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    Store store;
    fillStore(store);

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), moreThanFullQuantity, &player);

    REQUIRE(response == Response::lack_of_cargo);
}

TEST_CASE("rich player with empty ship buy half cargo than store have", "[Player][Ship][Store][sell]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    Store store;
    fillStore(store);

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), halfQuantity, &player);

    REQUIRE(response == Response::done);
    REQUIRE(player.getAvailableSpace() == fullCappacity - 1);
    REQUIRE(store.getCargoNum() == cargoTypes);
}

TEST_CASE("rich player with empty ship buy full single cargo from empty store", "[Player][Ship][Store][buy]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);
    Store store;
    fillStore(store);

    auto cargoExample = generateCargo(0);
    auto response = store.sell(cargoExample.get(), fullQuantity, &player);

    REQUIRE(response == Response::done);
    REQUIRE(player.getAvailableSpace() == fullCappacity - 1);
    REQUIRE(store.getCargoNum() == cargoTypes - 1);
}

TEST_CASE("rich player with empty ship with enough capacity buy full of all cargo from store", "[Player][Ship][Store][sell]") {
    auto ship = std::make_unique<Ship>(fullCappacity, 0, 0, "Ship", 0, std::vector<std::unique_ptr<Cargo>>{});
    Player player(std::move(ship), playerMoney);

    REQUIRE(player.getAvailableSpace() == fullCappacity);

    Store store;
    fillStore(store);

    for (int i = 0; i < cargoTypes; ++i) {
        auto cargoExample = generateCargo(i);
        auto response = store.sell(cargoExample.get(), fullQuantity, &player);
        REQUIRE(response == Response::done);
    }

    REQUIRE(player.getAvailableSpace() == 0);
    REQUIRE(store.getCargoNum() == 0);
}
