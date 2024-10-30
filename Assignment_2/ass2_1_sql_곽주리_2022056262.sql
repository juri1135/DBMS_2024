DROP DATABASE IF EXISTS DB_2022056262;

CREATE DATABASE DB_2022056262;

USE DB_2022056262;
-- userInfo table
CREATE TABLE `userInfo` (
    `userID` VARCHAR(10) NOT NULL,
    `password` VARCHAR(15) NOT NULL,
    `state` VARCHAR(5) NOT NULL,
    `balance` BIGINT NOT NULL,
    PRIMARY KEY (`userID`)
);

-- stock table
CREATE TABLE `stock` (
    `stockID` VARCHAR(10) NOT NULL,
    `curPrice` BIGINT NOT NULL,
    `prevClose` BIGINT NOT NULL,
    `chgPercent` DECIMAL(4, 1) NOT NULL,
    PRIMARY KEY (`stockID`)
);

-- orderBook table
CREATE TABLE `orderBook` (
    `orderID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `userID` VARCHAR(10) NOT NULL,
    `stockID` VARCHAR(10) NOT NULL,
    `state` VARCHAR(10) NOT NULL,
    `price` BIGINT NOT NULL,
    `quantity` BIGINT NOT NULL,
    `orderType` VARCHAR(10) NOT NULL,
    `priceType` VARCHAR(10) NOT NULL,
    `time` DATETIME NOT NULL,
    PRIMARY KEY (`orderID`),
    FOREIGN KEY (`userID`) REFERENCES `userInfo` (`userID`),
    FOREIGN KEY (`stockID`) REFERENCES `stock` (`stockID`)
);

-- transactionHistory table
CREATE TABLE `transactionHistory` (
    `transactionID` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `userID` VARCHAR(10) NOT NULL,
    `stockID` VARCHAR(10) NOT NULL,
    `orderID` INT UNSIGNED,
    `price` BIGINT NOT NULL,
    `orderType` VARCHAR(10) NOT NULL,
    `quantity` BIGINT NOT NULL,
    `priceType` VARCHAR(10) NOT NULL,
    `time` DATETIME NOT NULL,
    PRIMARY KEY (`transactionID`),
    FOREIGN KEY (`userID`) REFERENCES `userInfo` (`userID`),
    FOREIGN KEY (`orderID`) REFERENCES `orderBook` (`orderID`)
);

-- portfolio table
CREATE TABLE `portfolio` (
    `userID` VARCHAR(10) NOT NULL,
    `stockID` VARCHAR(10) NOT NULL,
    `quantity` BIGINT NOT NULL,
    `avgPrice` BIGINT NOT NULL,
    `chgPercent` DECIMAL(4, 1) NOT NULL,
    `evalPL` BIGINT NOT NULL,
    `tradQty` BIGINT NOT NULL,
    PRIMARY KEY (`userID`, `stockID`),
    FOREIGN KEY (`userID`) REFERENCES `userInfo` (`userID`),
    FOREIGN KEY (`stockID`) REFERENCES `stock` (`stockID`)
);
