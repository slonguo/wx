#ifndef LOGIN_DEFS_HPP
#define LOGIN_DEFS_HPP

#include <string>
#include "apps/3rd/json.hpp"
#include "apps/3rd/magic_enum.hpp"
#include "apis/cpp/protocol/v1/login.pb.h"

namespace login
{

using nlohmann::json;
using std::string;

using protocol::login::v1::AdminResp;
using protocol::login::v1::CommonHeaderResp;

enum MsgType
{
	MT_Init = 0,
	MT_LoginSucceed,
	MT_UserInfo,
	MT_Logout,
	MT_KickedOut,
	MT_Broadcast,
	// more msg types ...

	MT_Count,
};

enum RespCode
{
	OK = 0,

	// for user
	Invalid_Header_Request,

	User_Already_Registered,
	User_Already_Logged_In,
	User_Not_Found,
	User_Logout_Or_Kicked_Out,

	// for internal server Info
	Internal_Error_Init,
	Internal_DB_Error,

	// for admin operations
	Admin_Cmd_Init,
	Invalid_Admin_Command,
	Invalid_Config,
	Invalid_Subcommand_Or_Data,
};

enum SystemType
{
	ST_Unknown = 0,
	ST_iOS,
	ST_Android,
	ST_MacOS,
	ST_PC,
	ST_Web,
	ST_Count
};

enum DeviceType
{
	DT_Unknown = 0,
	DT_Apple_Init = 1000,
	DT_Huawei_Init = 2000,
	DT_Samsung_Init = 3000,
	DT_XiaoMi_Init = 4000,
	DT_Count = 100000,
};

enum ChannelType
{
	CT_Unknown = 0,
	CT_App_Store,
	CT_Google_Play,
	CT_Huawei_Store,
	// ...
	CT_Count,
};

enum LoginType
{
	LT_Unknown = 0,
	LT_Password,
	LT_Phone,
	LT_Session,
	// ...
	//LT_QR_Code,
	LT_Count,
};

enum UserStatus
{
	US_Init = 0,
	US_Deleted,
	US_Count,
};

struct UserSecureInfo
{
	string userName;
	string phoneNumber;
	string passwd;
	uint status;
};

struct LoginSucceedResp
{
	string token;
};

struct UserBasicInfo
{
	string userNick;
	string userName;
	int gender;
	string avatar;
	string signature;
};

struct UserLoginInfo
{
	string userName;
	uint systemType;
	uint deviceType;
	uint channelType;
	string deviceName;
	string deviceID;
	string IP;
	uint loginTime;
	uint logoutTime;
};

struct MsgItem
{
	MsgItem(MsgType type, string content) : type(type), content(content) {}
	MsgType type;
	string content;
};

inline string code2Msg(RespCode code)
{
	string msg = string(magic_enum::enum_name(code));
	for (auto &c : msg)
	{
		c == '_' ? c = ' ' : c = std::tolower(c);
	}
	return msg;
}

inline void SetCommonHeaderResp(CommonHeaderResp &header, RespCode code)
{
	header.set_code(uint32_t(code));
	header.set_message(code2Msg(code));
}

inline void SetAdminResp(AdminResp *resp, RespCode code)
{
	resp->mutable_header()->set_code(uint32_t(code));
	resp->mutable_header()->set_message(code2Msg(code));
}

inline bool IsValidMsgType(MsgType t) { return t >= MT_Init && t < MT_Count; }

inline bool IsValidLoginType(MsgType t) { return IsValidMsgType(t) && t != MT_Logout && t != MT_KickedOut; }

inline void to_json(json &j, const UserSecureInfo &b)
{
	// for security's sake, phone number and passwd are (partially) obscured
	string phoneNumber = b.phoneNumber.substr(0, 3) + "xxxx" + b.phoneNumber.substr(7, 11);
	j = json{
		{"user_name", b.userName},
		{"status", b.status},
		{"passwd", "xxxx"},
		{"phone_number", phoneNumber},
	};
}

inline void from_json(const json &j, UserSecureInfo &b)
{
	j.at("user_name").get_to(b.userName);
	j.at("status").get_to(b.status);
	j.at("passwd").get_to(b.passwd);
	j.at("phone_number").get_to(b.phoneNumber);
}

inline void to_json(json &j, const UserBasicInfo &b)
{
	j = json{
		{"user_nick", b.userNick},
		{"user_name", b.userName},
		{"gender", b.gender},
		{"avatar", b.avatar},
		{"signature", b.signature},
	};
}

inline void from_json(const json &j, UserBasicInfo &b)
{
	j.at("user_nick").get_to(b.userNick);
	j.at("user_name").get_to(b.userName);
	j.at("gender").get_to(b.gender);
	j.at("avatar").get_to(b.avatar);
	j.at("signature").get_to(b.signature);
}

inline void to_json(json &j, const UserLoginInfo &b)
{
	j = json{
		{"user_name", b.userName},
		{"system_type", b.systemType},
		{"device_type", b.deviceType},
		{"channel_type", b.channelType},
		{"device_name", b.deviceName},
		{"device_id", b.deviceID},
		{"ip", b.IP},
	};
}

inline void from_json(const json &j, UserLoginInfo &b)
{
	j.at("user_name").get_to(b.userName);
	j.at("system_type").get_to(b.systemType);
	j.at("device_type").get_to(b.deviceType);
	j.at("channel_type").get_to(b.channelType);
	j.at("device_name").get_to(b.deviceName);
	j.at("device_id").get_to(b.deviceID);
	j.at("ip").get_to(b.IP);
}

inline void to_json(json &j, const LoginSucceedResp &l)
{
	j = json{
		{"token", l.token},
	};
}

inline void from_json(const json &j, LoginSucceedResp &l)
{
	j.at("token").get_to(l.token);
}

} // namespace login

#endif