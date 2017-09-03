# ************************************************************
# Sequel Pro SQL dump
# Version 4096
#
# http://www.sequelpro.com/
# http://code.google.com/p/sequel-pro/
#
# Host: 10.8.0.1 (MySQL 10.1.16-MariaDB-1~trusty)
# Database: nmr
# Generation Time: 2017-09-03 09:48:31 +0000
# ************************************************************


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


# Dump of table nmr_data
# ------------------------------------------------------------

CREATE TABLE `nmr_data` (
  `tag_id` varchar(16) NOT NULL,
  `r_id` tinyint(1) NOT NULL,
  `time` timestamp(2) NOT NULL DEFAULT CURRENT_TIMESTAMP(2) ON UPDATE CURRENT_TIMESTAMP(2),
  `row_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`row_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



# Dump of table nmr_env
# ------------------------------------------------------------

CREATE TABLE `nmr_env` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` timestamp(2) NOT NULL DEFAULT CURRENT_TIMESTAMP(2),
  `sensor` tinytext,
  `value` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



# Dump of table nmr_index
# ------------------------------------------------------------

CREATE TABLE `nmr_index` (
  `id` smallint(4) NOT NULL AUTO_INCREMENT,
  `tag_id` varchar(16) DEFAULT NULL,
  `sex` varchar(1) DEFAULT NULL,
  `colony` varchar(20) NOT NULL,
  `dob` date DEFAULT NULL,
  `dob_estimated_flag` varchar(1) DEFAULT NULL,
  `notes` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `tag_id` (`tag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



# Dump of table nmr_manual_obs
# ------------------------------------------------------------

CREATE TABLE `nmr_manual_obs` (
  `id` smallint(4) unsigned NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `behaviour` varchar(512) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table nmr_nest_distances
# ------------------------------------------------------------

CREATE TABLE `nmr_nest_distances` (
  `date` date NOT NULL,
  `reader_a` smallint(4) NOT NULL,
  `reader_b` smallint(4) NOT NULL,
  `distance` int(8) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table nmr_readers
# ------------------------------------------------------------

CREATE TABLE `nmr_readers` (
  `r_id` tinyint(1) NOT NULL,
  `serial_no` mediumint(7) DEFAULT NULL,
  `location` tinyint(1) NOT NULL,
  `reader_notes` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`r_id`),
  UNIQUE KEY `serial_no` (`serial_no`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



# Dump of table nmr_records
# ------------------------------------------------------------

CREATE TABLE `nmr_records` (
  `id` smallint(4) NOT NULL,
  `mass` decimal(5,2) DEFAULT NULL,
  `date` date NOT NULL,
  `status` varchar(64) DEFAULT NULL,
  `rank` varchar(64) DEFAULT NULL,
  `teats` varchar(4) DEFAULT NULL,
  `f_genitalia` varchar(16) DEFAULT NULL,
  PRIMARY KEY (`id`,`date`),
  CONSTRAINT `nmr_records_ibfk_1` FOREIGN KEY (`id`) REFERENCES `nmr_index` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;




/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
