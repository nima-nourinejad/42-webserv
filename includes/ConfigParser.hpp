/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 19:39:10 by akovalev          #+#    #+#             */
/*   Updated: 2025/02/06 18:06:48 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ServerBlock.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <sstream>

enum class TokenType
{
	KEY_VALUE,
	KEY_MULTI_VALUE,
	OPEN,
	CLOSE,
	SEMICOLON,
	COMMENT,
	END_OF_FILE,
	UNKNOWN
};

struct Token
{
	TokenType					type;
	std::string					key;
	std::vector<std::string>	values;  

	Token(TokenType t, const std::string& k, const std::string& v) 
		: type(t), key(k), values{v} {}

	Token(TokenType t, const std::string& k, const std::vector<std::string>& vals)
		: type(t), key(k), values(vals) {}

	Token(TokenType t, const std::string& val) 
		: type(t), key(""), values{val} {}
};

class ConfigParser
{
	private:
		std::vector<ServerBlock>	_server_blocks;
		std::vector<Token>			_tokens;
	public:
		ConfigParser();
		~ConfigParser();

		std::vector<ServerBlock>	parseConfig(std::ifstream& filepath);
		std::vector<ServerBlock>	getServerBlocks() const;
		void						tokenize(std::vector<Token>& tokens, std::ifstream& filepath);
		void						parseServerBlock(size_t& index);
		void						parseLocationBlock(size_t& index);
		void						printServerConfig();
		void						unexpectedToken(size_t i);
};

#endif
