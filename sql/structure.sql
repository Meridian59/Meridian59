CREATE DATABASE IF NOT EXISTS `meridian` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `meridian`;

CREATE TABLE IF NOT EXISTS `money_created` (
  `idmoney_created` int(11) NOT NULL AUTO_INCREMENT,
  `money_created_amount` int(11) NOT NULL,
  `money_created_time` datetime NOT NULL,
  PRIMARY KEY (`idmoney_created`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `player_damaged` (
  `idplayer_damaged` int(11) NOT NULL AUTO_INCREMENT,
  `player_damaged_who` varchar(45) NOT NULL,
  `player_damaged_attacker` varchar(45) NOT NULL,
  `player_damaged_aspell` int(11) NOT NULL,
  `player_damaged_atype` int(11) NOT NULL,
  `player_damaged_applied` int(11) NOT NULL,
  `player_damaged_original` int(11) NOT NULL,
  `player_damaged_weapon` varchar(45) NOT NULL,
  `player_damaged_time` datetime NOT NULL,
  PRIMARY KEY (`idplayer_damaged`)
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `player_logins` (
  `idplayer_logins` int(11) NOT NULL AUTO_INCREMENT,
  `player_logins_account_name` varchar(45) NOT NULL,
  `player_logins_character_name` varchar(45) NOT NULL,
  `player_logins_IP` varchar(45) NOT NULL,
  `player_logins_time` datetime NOT NULL,
  PRIMARY KEY (`idplayer_logins`)
) ENGINE=InnoDB AUTO_INCREMENT=35 DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS `player_money_total` (
  `idplayer_money_total` int(11) NOT NULL AUTO_INCREMENT,
  `player_money_total_time` datetime NOT NULL,
  `player_money_total_amount` int(11) NOT NULL,
  PRIMARY KEY (`idplayer_money_total`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=latin1;
