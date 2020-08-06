#include <iostream>
#include <string>
#include <vector>

using namespace std;

//1. �ȶ���ö����
enum Token {
	nul = 0,
	paragraph = 1,
	href = 2,
	ul = 3,
	ol = 4,
	li = 5,
	em = 6,
	strong = 7,
	hr = 8,    // ˮƽ�ָ���
	image = 9,
	quote = 10,
	h1 = 11,
	h2 = 12,
	h3 = 13,
	h4 = 14,
	h5 = 15,
	h6 = 16,
	blockcode = 17,
	code = 18,
};
// HTMLǰ�ñ�ǩ
const std::string frontTag[] = {
	"", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />",
	"", "<blockquote>", "<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>",
	"<pre><code>", "<code>" };
// HTML ���ñ�ǩ
const std::string backTag[] = {
	"", "</p>", "", "</ul>", "</ol>", "</li>", "</em>",
	"</strong>", "", "", "</blockquote>", "</h1>", "</h2>",
	"</h3>", "</h4>", "</h5>", "</h66>", "</code></pre>", "</code>" };
 
// ������������
struct Node{
	// �﷨����
	int _type;
	// ���ӽڵ�
	vector<Node*> _child;
	// ���ݣ�ͼƬ�����ӣ�����+��ַ����
	// elem[0]: ������ʾ������
	// elem[1]: ������ַ��·��
	string elem[2];

	// ���ڵ�
	Node(int type)
		:_type(type)
	{}
};
// ������
class markdownParser{
public:
	markdownParser(const string& filename)
		:_root(new Node(nul))
		, _filename(filename)
	{}
	void transform(){
		// ���ļ�
		ifstream fin(_filename);
		if (!fin.is_open()){
			cout << "file: " << _filename << "open failed" << endl;
		}
		bool inblock = false;// �ڴ������
		// ��ȡ����
		string rowStr;
		while (!fin.eof()){
			getline(fin, rowStr);
		
			// Ԥ�����������׿ո�, һ����ͷɾ����ͷɾ���ӣ� ѡȡ
			const char* start = processStr(rowStr.c_str());

			// �ж��Ƿ�Ϊ�����ݣ� 
			if (!inblock && start == nullptr)
				continue; // ��Ϊ�գ� ֱ�Ӵ�����һ��

			// �ж��Ƿ�Ϊˮƽ�ָ��ߣ� ����ǣ��򴴽�ˮƽ�ָ��߽��
			if (!inblock && isCutLine(start)){
				_root->_child.push_back(new Node(hr));
				continue;
			}
			// �﷨���� �������ݽ�������,�õ���ǰ�ַ��������ĵ�һ���﷨,���õ��﷨��ǩ�������
			// ���Խ����������׵��﷨, �������,����Ҫ�߲���߽���
			pair<int, const char*> typeRet = parserType(start); // ����֮������͵Ľ��


			// �����﷨���, 
			// �жϴ������
			if (typeRet.first == blockcode){
				// �ж��Ǵ�������ʼ���ǽ���
				if (!inblock){// ֮ǰ�ڴ������, 
					//������Ϊ��������ʼ,  �򴴽��������
					_root->_child.push_back(new Node(blockcode));
				}
				// ���֮ǰ���ڴ������, �������ڴ�������λ��, ����Ҫ�����µĴ������
				inblock = !inblock; 
				continue;
				// ���ݻ�û�Ž�ȥ
			}

			// �ж��Ƿ�Ϊ������еĴ���
			if (inblock){// ����Ǵ�����е�
				_root->_child.back()->elem[0] += rowStr;// �����ı��Դ�. �������ַ���,����ʲô����ʲô
				_root->_child.back()->elem[0] += '\n'; // ���м�'\n'
				continue;// ����������һ�е�����
			}

			// ������
			if (typeRet.first == paragraph){
				// ����һ��������ŵ����ӵ����һ��λ��, ���ַ�����
				_root->_child.push_back(new Node(paragraph));
				insert(_root->_child.back(), typeRet.second);
				continue;
			}

			// ����Ǳ���
			if (typeRet.first >= h1 && typeRet.first <= 6){
				// ����������
				_root->_child.push_back(new Node(typeRet.first)); // �Ǽ����ʹ�������
				// �����������
				_root->_child.back()->elem[0] = typeRet.second;
				continue;
			}

			// �����б� ���������Ƿ����б����е�, �������б��ÿһ�����һ�����ǩ��,
			// ���ж����һ�������Ƿ��������б�, �����, ��ǰ���������б����
			if (typeRet.first == ul){
				// �ж��Ƿ�Ϊ�����б�ĵ�һ��
				// �ĵ��Ŀ�ʼ(Ϊ��)�����﷨���д�ʱ�����һ����㲻��������(˵�����Խ���������Ŀ϶��������б�ĵ�һ�����)
				if (_root->_child.empty() || _root->_child.back()->_type != ul){
					// ���������б�
					_root->_child.push_back(new Node(ul));// ul�������б�
				}
				// �������б�����б��ӽڵ�
				Node* UNode = _root->_child.back();
				UNode->_child.push_back(new Node(li));// li���б�
				// ���б��ӽڵ�������ݵĲ���
				insert(UNode->_child.back(), typeRet.second);
			}

		}
		
		cout << "transfrom finished" << endl;

		

		// չ���﷨���� ����html�ĵ�
	}
	// ���ַ��������ݲ���
	void insert(Node* curNode, const char* str){}

	// ���������﷨, �Ƚ�������
	// ����ֵ: �﷨���� + �﷨��Ӧ���ݵ���ʼλ��
	pair<int, const char*> parserType(const char* str){
		// ��������: # + �ո�
		const char* ptr = str;
		int titleNum = 0;// ���⼶��
		while (*ptr && *ptr == '#'){
			// ��ͷ������#��ʼ, ���Ǳ���, ͳ���Ǽ�������
			++ptr;
			++titleNum;
		}
		if (*ptr == ' ' && titleNum > 0 && titleNum < 6){
			// �����ո�, �����һ������, �򴴽���������,�Լ��ո�֮�������(�ո�+1)
			return make_pair(h1 + titleNum - 1, ptr + 1); // h1(����1)�������11
		}
		// ���Ǳ���, �����½���, ��ָ���λ = ��ʼλ��
		ptr = str;

		// 2. ���Ƿ��Ǵ���� '''��������'''
		if (strncmp(ptr, "'''", 3) == 0){
			return make_pair(blockcode, ptr + 3);
		}
		// 3. �����б�, �ж�ǰ���ַ��Ƿ��Ǻ��+�ո�(- )
		if (strncmp(ptr, "- ", 2) == 0){
			return make_pair(ul, ptr + 2);
		}
		// 4. �����б�, �����ַ� + '.' + '�ո�'
		if (*ptr >= '0' && *ptr <= '9'){
			// �Ȱ�����ȫ��������
			while (*ptr && *ptr >= '0' && *ptr <= '9'){// ��������0~9֮��
				++ptr;
			}
			if (*ptr && *ptr == '.'){
				++ptr;
				if (*ptr == ' '){
					// ˵������һ�������б���
					return make_pair(ol, ptr + 1);// �����Ѿ��ߵ��ո���, ֻ���1�͵�����������������
				}
			}
			// ������Ҫ��, ������
			ptr = str;
		}

		// 5. ����, ">" + �ո�
		if (strncmp(ptr, "> ", 2) == 0){
			return make_pair(quote, ptr + 2);
		}
		// �����﷨ ͳһ����Ϊ������
		return make_pair(paragraph, ptr);
	}

	// �ж��Ƿ�Ϊˮƽ�ָ��� ---(���ߴ���3)  �ڴ������Ŵ���,
	bool isCutLine(const char* str){
		int cnt = 0;
		while (*str && *str == '-'){
			++str;
			++cnt;
		}
		return cnt >= 3;
	}

	const char* processStr(const char* str){
		while (*str){
			if (*str == ' ' || *str == '\t')// �пո��tab�� ������
				++str;
			else 
				break; // ����ͣ��
		}
		if (*str == '\0')
			return nullptr; // �ߵ�ĩβ
		return str;
	}

private:
	// �﷨�������
	Node* _root;
	// �ļ���
	string _filename;
	// ���html�ĵ�����
	string _contents;
};