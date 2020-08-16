#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "markdownParser.h"
using namespace std;

/*
项目： 组一个简单的是Markdown语法的解析器
呈现结果： 把输入的Markdown文档最终转换为html文档
常用语法：段落，标题，图片，连接，代码块，行内代码，引用，列表（有序，无序），粗体，斜体
Markdown：                                            html:
	段落：没有前置语法标题							  <p>
	标题： #..#####(1-6级)							  <h1>,<h2>,<h3>,<h4>,<h5>,<h6>
	图片: ![图片名称](图片地址)						  <img>
	链接: [名称](链接地址)							  <a href = > </a>
	代码块: '''代码块'''(可能有多行)					  <pre><code>
	行内代码: '行内代码'								  <code>
	引用: >引用内容									  <blockquote>
	无序列表: -无序列表内容(可能有多行)				  <ul><li>
	有序列表: 数字.有序列表内容(可能有多行)				  <ol><li>
	粗体: **粗体内容**								  <strong>
	斜体: _斜体内容_									  <em>
解析过程: 以行为单位进行解析
	1. 解析行首语法(无行首语法,则暂时全部解析为段落)
	2. 根据语法创建节点(N叉树的结点), 节点结构:语法类型,孩子节点指针数组,实际的内容(显示的内容+链接/地址)
	3. 结点的插入
	4. N叉树展开,把N叉树节点的内容转为html文档
节点插入:
	直接插入:
		标题(直接插入)
		代码块(直接插入)
	需要再次逐字符解析:(行内代码可能包含了图片,链接,行内代码,粗体,斜体)
		段落
		引用
		无序列表
		有序列表
	解析的过程中: 遇到非行首语法, 创建孩子节点, 新的内容存放点傲孩子节点中
		纯文本: nul节点
			遇到非行首语法: 再去创建新的孩子节点
			纯文本: nul节点
遍历顺序:
	深度优先
	每一层从左向右遍历
			
	


*/
// 构建树
markdownParser::markdownParser(const string& filename)
		:_root(new Node(nul))
		, _filename(filename)
	{}
void markdownParser::transform(){
		// 打开文件
		ifstream fin(_filename);
		if (!fin.is_open()){
			cout << "file: " << _filename << "open failed" << endl;
		}
		bool inblock = false;// 在代码块里
		// 读取内容
		string rowStr;
		while (!fin.eof()){
			getline(fin, rowStr);

			// 预处理：处理行首空格, 一般是头删，但头删复杂； 选取
			const char* start = processStr(rowStr.c_str());

			// 判断是否为空内容， 
			if (!inblock && start == nullptr)
				continue; // 若为空， 直接处理下一行

			// 判断是否为水平分割线， 如果是，则创建水平分割线结点
			if (!inblock && isCutLine(start)){
				_root->_child.push_back(new Node(hr));
				continue;
			}
			// 语法解析 根据内容解析类型,拿到当前字符串遇到的第一个语法,再拿到语法标签后的内容
			// 可以解析的是行首的语法, 如果不是,则需要边插入边解析
			pair<int, const char*> typeRet = parserType(start); // 解析之后的类型的结果


			// 创建语法结点, 
			// 判断代码块结点
			if (typeRet.first == blockcode){
				// 判断是代码块的起始还是结束
				if (!inblock){// 之前在代码块外, 
					//则现在为代码块的起始,  则创建代码块结点
					_root->_child.push_back(new Node(blockcode));
				}
				// 如果之前是在代码块里, 则现在在代码块结束位置, 不需要创建新的代码块结点
				inblock = !inblock;
				continue;
				// 内容还没放进去
			}

			// 判断是否为代码块中的代码
			if (inblock){// 如果是代码块中的
				_root->_child.back()->elem[0] += rowStr;// 按纯文本对待. 加整个字符串,读到什么就是什么
				_root->_child.back()->elem[0] += '\n'; // 多行加'\n'
				continue;// 继续处理下一行的内容
			}

			// 段落结点
			if (typeRet.first == paragraph){
				// 创建一个段落结点放到孩子的最后一个位置, 逐字符插入
				_root->_child.push_back(new Node(paragraph));
				insert(_root->_child.back(), typeRet.second);
				continue;
			}

			// 如果是标题
			if (typeRet.first >= h1 && typeRet.first <= 6){
				// 创建标题结点
				_root->_child.push_back(new Node(typeRet.first)); // 是几级就创建几级
				// 插入标题内容
				//_root->_child.back()->elem[0] = typeRet.second;
				// 如果标题中有嵌套的网址
				insert(_root->_child.back(), typeRet.second);
				continue;
			}

			// 无序列表 真正内容是放在列表结点中的, 把无序列表的每一项放在一个大标签里,
			// 先判断最后一个孩子是否是无序列表, 如果是, 则当前就是无序列表的项
			if (typeRet.first == ul){
				// 判断是否为无序列表的第一项
				// 文档的开始(为空)或者语法树中此时的最后一个结点不是无序结点(说明可以接下来插入的肯定是无序列表的第一个结点)
				if (_root->_child.empty() || _root->_child.back()->_type != ul){
					// 创建无序列表
					_root->_child.push_back(new Node(ul));// ul是无序列表
				}
				// 给无序列表加入列表子节点
				Node* UNode = _root->_child.back();
				UNode->_child.push_back(new Node(li));// li是列表
				// 给列表子节点进行内容的插入
				insert(UNode->_child.back(), typeRet.second);
			}

			// 有序列表,
			if (typeRet.first == ol){
				if (_root->_child.empty() || _root->_child.back()->_type != ol){
					_root->_child.push_back(new Node(ol));
				}
				Node* oNode = _root->_child.back();
				oNode->_child.push_back(new Node(li));
				insert(oNode->_child.back(), typeRet.second);
				continue;
			}

			// 引用结点
			if (typeRet.first == quote){
				// 创建引用结点
				_root->_child.push_back(new Node(quote));
				insert(_root->_child.back(), typeRet.second);
			}
		}

		cout << "transfrom finished" << endl;

		// 展开语法树， 生产html文档
		// 从根开始走深度优先
		dfs(_root);
	}

	// 把语法树转化为html源代码(文档的body)
	// 树的遍历使用深度优先(当前这个结点应该做什么事情, 后序也是,相当于并行递归)
	void markdownParser::dfs(Node* root){
		// 首先不能为空, 孩子为空就不能走递归了,
		// 插入前置标签
		_contents += frontTag[root->_type];

		// 插入结点内容
		// 特殊内容
		// 网址: 
		if (root->_type == href){
			_contents += "<a href=\"";
			_contents += root->elem[1];
			_contents += "\">";
			_contents += root->elem[0];
			_contents += "</a>";
		}
		// 图片:
		if (root->_type == image){
			_contents += "<img alt=\"";
			_contents += root->elem[0];
			_contents += "\" src=\"";
			_contents += root->elem[1];
			_contents += "\" />";
		}
		else{
			_contents += root->elem[0];
		}
		// 孩子节点处理(边界就是走到空或者没有孩子时)
		for (Node* ch : root->_child){
			dfs(ch);
		}
		// 插入后置标签
		_contents += backTag[root->_type];
	}


	// 逐字符进行内容插入, 先判断是不是特殊结点, 如果不是就进行纯文本插入
	void markdownParser::insert(Node* curNode, const char* str){
		// 定义是不是行内代码的标记, 如果是代码块是不需要解析的,
		bool incode = false;// 行内代码标记
		bool instrong = false; // 粗体标记
		bool inem = false; // 斜体标记
		bool indel = false; // 下划线标记
		bool inannotation = false; // 注释标记

		int len = strlen(str);
		// 有很多子节点(文本,有序,无序,..) 可以把解析的当做外部孩子节点, 
		// 如果解析的内容为纯文本, 可以存入纯文本结点中
		// 首先创建一个纯文本孩子节点
		curNode->_child.push_back(new Node(nul));
		for (int i = 0; i < strlen(str); ++i){
			// 进行插入和解析
			// 放入行内代码
			if (str[i] == '`'){
				if (incode){
					// 如果行内代码结束, 则需要创建一个新的孩子节点去存放后序的内容
					curNode->_child.push_back(new Node(nul));
				}
				else{
					// 如果不是行内代码 就需要创建行内代码节点
					curNode->_child.push_back(new Node(code));
				}
				incode = !incode;
				continue;
			}
			// 不管是粗体还是斜体 , 只要为一, 当前行的最后一个孩子节点肯定为粗体结点或者斜体结点(**/_),此时不管斜体/粗体后面的纯文本
			// **粗体**  
			// 出现**且不在行内代码中, 则为粗体
			if (str[i] == '*' && i + 1 < len && str[i + 1] == '*' && !incode){
				// 是否已经在粗体中了, 是的话 需要创建nul存放新的结点
				if (instrong){
					// 如果粗体结束, 则创建新的结点
					curNode->_child.push_back(new Node(nul));
				}
				else{
					// 如果不在粗体, 则创建一个粗体结点
					curNode->_child.push_back(new Node(strong));
					// 创建一个粗体的孩子节点
					curNode->_child.back()->_child.push_back(new Node(nul));
				}
				instrong = !instrong;
				// 跳过粗体语法
				++i;
				continue;
			}

			// _斜体_
			// 遇到斜体且不再行内代码中,
			if (str[i] == '_' && incode){
				if (inem){
					curNode->_child.push_back(new Node(nul));
				}
				else{
					curNode->_child.push_back(new Node(em));
					curNode->_child.back()->_child.push_back(new Node(nul));
				}
				inem = !inem;
				continue;
			}

			// 图片 : ![图片名称](地址)
			if (str[i] == '!' && i + 1 < len && str[i + 1] == '['){
				// 先获取图片名称, 不能和地址放在同一个地方 可用数组
				// 创建图片结点
				curNode->_child.push_back(new Node(image));
				Node* iNode = curNode->_child.back();
				i += 2;
				// 走到图片名称正文, i有效且不能等于右边的']'
				for (; i < len && str[i] != ']'; ++i){
					iNode->elem[0] += str[i];
				}
				// 存放图片地址, 此时在']'处, 需要先+2到地址
				i += 2;
				for (; i < len && str[i] != ')'; ++i){
					iNode->elem[1] += str[i];
				}
				// 图片处理完了, 处理图片后面的内容, 也许创建新的结点
				curNode->_child.push_back(new Node(nul));
				continue;
			}


			// 链接: [链接名称](网址)
			// 出现左'[' 且不再行内代码中, 则为链接
			if (str[i] == '[' && incode){
				Node* hNode = NULL;
				// 判断是否为粗体中嵌套的链接, 不管是粗体还是斜体都是当前行首节点的最后一个孩子
				if (instrong || inem){
					// 给粗体内容放入孩子节点 创建粗体(当前行首的最后一个孩子节点)的孩子节点
					curNode->_child.back()->_child.push_back(new Node(href)); 
					hNode = curNode->_child.back()->_child.back();// 当前结点最后一个孩子(粗体)的孩子节点
				}
				else{
					// 非嵌套,则直接在行首节点创建链接结点
					curNode->_child.push_back(new Node(href));
					Node* hNode = curNode->_child.back();

				}
				
				++i;
				// 存放链接名称
				for (; i < len && str[i] != ']'; ++i){
					hNode->elem[0] += str[i];
				}
				i += 2;
				// 存放链接地址
				for (; i < len && str[i] != ')'; ++i){
					hNode->elem[1] += str[i];
				}
				// 链接后面还有值, 需要创建新的结点存放链接后面的内容
				// 此时就需要判断是否要存放到粗体中(成为粗体的孩子节点)
				if (instrong || inem){
					curNode->_child.back()->_child.push_back(new Node(nul));
				}
				else{// 否则就是给行首放nul结点
					curNode->_child.push_back(new Node(nul));
				}
				continue;
			}

			// 下划线 <u> ...</u>
			// 下划线开始
			if (strncmp(str + i, "<u>", 3) == 0){
				// 创建一个下划线节点
				curNode->_child.push_back(new Node(u)); // 在后面内容加下划线
				i += 2;// 因为for循环本来就会往后再走一步, 这里需要少走一步
				continue;
			}
			// 下划线结束
			else if(strncmp(str + i, "</u>", 4) == 0){
				curNode->_child.push_back(new Node(nul));// 后面内容为纯文本
				i += 3;
				continue;
			}
			 
			// 删除线 ~~...~~
			if (strncmp(str + i, "~~", 2) == 0){
				if (indel){
					// 如果之前是一个删除线, 存放删除线后的内容
					curNode->_child.push_back(new Node(nul));
				}
				else{
					// 如果之前不是删除线, 则创建删除线标记
					curNode->_child.push_back(new Node(del));
				}
				indel = !indel;
				++i;
				continue;
			}

			// 注释 <!--...-->
			if (strncmp(str + i, "<!--", 4) == 0){
				curNode->_child.push_back(new Node(ano));
				inannotation = true;
				i += 3;
				continue;
			}
			else if(inannotation && strncmp(str + i, "-->", 3) == 0){
				// 注释标志结束位置
				curNode->_child.push_back(new Node(nul));
				inannotation = false;
				i += 2;
				continue;
			}

			// 普通的纯文本
			if (instrong || inem){
				curNode->_child.back()->_child.back()->elem[0] += str[i];
			}
			else{
				curNode->_child.back()->elem[0] += str[i];
			}
		}
		// 行内代码
	}

	// 解析行首语法, 先解析标题
	// 返回值: 语法类型 + 语法对应内容的起始位置
	pair<int, const char*> markdownParser::parserType(const char* str){
		// 解析代码块时, 可能遇到空行, 预处理之后会是空指针, 是没有办法解析(解引用的, 会出现异常, 这个问题只会出现在代码块中)
		if (str == nullptr){
			return make_pair(nul, nullptr);// 如果是空指针,不用解析了, 返回内容为空的空指针;
			// 这样就不会删除代码段中的空行 而是原封不动的放进去 后面继续写数据
		}
		// 解析标题: # + 空格
		const char* ptr = str;
		int titleNum = 0;// 标题级数
		while (*ptr && *ptr == '#'){
			// 若头部是以#开始, 则是标题, 统计是几级标题
			++ptr;
			++titleNum;
		}
		if (*ptr == ' ' && titleNum > 0 && titleNum < 6){
			// 遇到空格, 则就是一个标题, 则创建返回类型,以及空格之后的内容(空格+1)
			return make_pair(h1 + titleNum - 1, ptr + 1); // h1(标题1)本身就是11
		}
		// 不是标题, 需重新解析, 把指针归位 = 起始位置
		ptr = str;

		// 2. 看是否是代码块 '''代码内容'''
		if (strncmp(ptr, "'''", 3) == 0){
			return make_pair(blockcode, ptr + 3);
		}
		// 3. 无序列表, 判断前两字符是否是横杠+空格(- )
		if (strncmp(ptr, "- ", 2) == 0){
			return make_pair(ul, ptr + 2);
		}
		// 4. 有序列表, 数字字符 + '.' + '空格'
		if (*ptr >= '0' && *ptr <= '9'){
			// 先把数字全部遍历完
			while (*ptr && *ptr >= '0' && *ptr <= '9'){// 存在且在0~9之间
				++ptr;
			}
			if (*ptr && *ptr == '.'){
				++ptr;
				if (*ptr == ' '){
					// 说明这是一个有序列表了
					return make_pair(ol, ptr + 1);// 这里已经走到空格了, 只需加1就到我们真正的内容了
				}
			}
			// 不符合要求, 则重置
			ptr = str;
		}

		// 5. 引用, ">" + 空格
		if (strncmp(ptr, "> ", 2) == 0){
			return make_pair(quote, ptr + 2);
		}
		// 其他语法 统一解析为段落结点
		return make_pair(paragraph, ptr);
	}

	// 判断是否为水平分割线 ---(横线大于3)  在代码块外才处理,
	bool markdownParser::isCutLine(const char* str){
		int cnt = 0;
		while (*str && *str == '-'){
			++str;
			++cnt;
		}
		return cnt >= 3;
	}
	// 去掉行首的空格
	const char* markdownParser::processStr(const char* str){
		while (*str){
			if (*str == ' ' || *str == '\t')// 有空格或tab键 往后走
				++str;
			else
				break; // 否则停掉
		}
		if (*str == '\0')
			return nullptr; // 走到末尾
		return str;
	}

	string markdownParser::getContents() const{
		return _contents;
	}

	// 最终html文档由 head + 正文 + end三部分组成
	void markdownParser::generateHtml(){
		std::string head = 
		"<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>Markdown</title>\
        <link rel=\"stylesheet\" href=\"github-markdown.css\">\
		</head><body><article class=\"markdown-body\">";

		std::string end = "</article></body></html>";

		ofstream fout("out/markdown.html");
		// 输出头,内容和尾
		fout << head << _contents << end;
	}

	// 销毁
	void  markdownParser::destory(Node* root){
		if (root){
			// 先销毁孩子 再释放空间
			for (Node* ch : root->_child)
				destory(ch);
			delete root;
		}
	}
	// 析构
	markdownParser::~markdownParser(){
		if (_root){
			destory(_root);
		}
	}
