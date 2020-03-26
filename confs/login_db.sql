
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for user_basic_tab
-- ----------------------------
DROP TABLE IF EXISTS `user_basic_tab`;
CREATE TABLE `user_basic_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `user_nick` varchar(32) NOT NULL DEFAULT '',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `avatar` varchar(255) NOT NULL DEFAULT '',
  `signature` varchar(64) NOT NULL DEFAULT '',
  `ctime` int(11) NOT NULL DEFAULT '0',
  `mtime` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for user_login_tab
-- ----------------------------
DROP TABLE IF EXISTS `user_login_tab`;
CREATE TABLE `user_login_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `system_type` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `device_type` int(11) unsigned NOT NULL DEFAULT '0',
  `channel_type` int(11) unsigned NOT NULL DEFAULT '0',
  `device_name` varchar(16) NOT NULL DEFAULT '',
  `device_id` varchar(64) NOT NULL DEFAULT '',
  `ip` char(16) NOT NULL DEFAULT '',
  `login_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `logout_time` bigint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_user_login` (`user_name`,`login_time`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for user_register_tab
-- ----------------------------
DROP TABLE IF EXISTS `user_register_tab`;
CREATE TABLE `user_register_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `system_type` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `device_type` int(11) unsigned NOT NULL DEFAULT '0',
  `channel_type` int(11) unsigned NOT NULL DEFAULT '0',
  `device_name` varchar(16) NOT NULL DEFAULT '',
  `device_id` varchar(64) NOT NULL DEFAULT '',
  `ip` char(16) NOT NULL DEFAULT '',
  `ctime` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for user_secure_tab
-- ----------------------------
DROP TABLE IF EXISTS `user_secure_tab`;
CREATE TABLE `user_secure_tab` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_name` varchar(16) NOT NULL,
  `phone_number` char(11) NOT NULL COMMENT '手机号，只考虑86的',
  `passwd` char(32) NOT NULL COMMENT 'md5串，实际中用bcrypt',
  `status` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ctime` int(11) unsigned NOT NULL DEFAULT '0',
  `mtime` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_uniq_name` (`user_name`) USING BTREE,
  UNIQUE KEY `idx_uniq_phone` (`phone_number`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8mb4;

SET FOREIGN_KEY_CHECKS = 1;
