/*
Adam Jereczek 149448 gr.7.
Struktury Baz Danych.
Rekordy pliku: Zbiory liczb rzeczywistych(max 15). Uporz¹dkowane wed³ug rozpiêtoœci zbioru.
Scalanie naturalne. Schemat 3+1.
*/

#include<iostream>
#include<fstream>
#include<vector>
#include<random>
#include<functional>
#include<string>
#include<sstream>
#include<regex>
#include<type_traits>


#define MIN_RECORD_LENGTH	1
#define MAX_RECORD_LENGTH	15
#define LOW_BOUND			-1000
#define HIGH_BOUND			1000
#define LOW_GENERATED		500
#define HIGH_GENERATED		1000
#define SITE_SIZE			100
#define TAPES_AMOUNT		4		//(3+1)


namespace Jereczek
{
	void cls()
	{
		std::system("cls");
	}


	template<typename T>
	class MyRand
	{
	public:
		MyRand(const T &low, const T &high)
			: r(std::bind(std::uniform_real_distribution<>(low, high), std::default_random_engine(std::random_device{}())))
		{
		}

		T operator()()
		{
			return this->r();
		}

	private:
		std::function<T()> r;
	};


	template<typename T>
	class Record
	{
	public:
		Record()
		{
			this->numbers = std::vector<T>(MAX_RECORD_LENGTH);
			this->size = 0;
			//this->generate(size);
		}

		Record(const std::string &s)
		{
			this->numbers = std::vector<T>(MAX_RECORD_LENGTH);
			this->size = 0;
			std::stringstream ss(s);
			std::string buffer;
			bool failed = false;
			std::regex isNumber("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?");

			while (this->size < MAX_RECORD_LENGTH)
			{
				ss >> buffer;
				if (buffer == "nan")
				{
					if (std::is_same<T, double>::value)
						this->numbers[this->size++] = std::numeric_limits<double>::quiet_NaN();
				}
				else if (std::regex_match(buffer, isNumber))
				{
					this->numbers[this->size++] = std::stod(buffer);
				}
				else
				{
					failed = true;
					break;
				}
				if (ss.eof())
				{
					break;
				}

			}

			for (unsigned short i = this->size; i < MAX_RECORD_LENGTH; ++i)
			{
				if (std::is_same<T, double>::value)
					this->numbers[i] = std::numeric_limits<double>::quiet_NaN();
			}

			if (failed)
			{
				this->validation();
			}
		}

		~Record()
		{

		}

		void validation()
		{
			std::string sign;
			bool quit = false;

			//Jereczek::cls():
			std::cout << "Napotkano blad podczas parsowania danych!\n";
			std::cout << "Aktualnie wczytane dane:\n";
			std::cout << "\t" << *this;

			if (this->size == 0)
			{
				std::cout << "Usuwanie rekordu, brak zawartosci!\nNacisnij ENTER!";
				std::getline(std::cin, sign);
				//Jereczek::cls():
			}
			else
			{
				while (!quit)
				{
					std::cout << "Zachowac rekord? (y/n) ";
					std::getline(std::cin, sign);
					if (sign == "y" || sign == "Y")
					{
						quit = true;
					}
					else if (sign == "n" || sign == "N")
					{
						quit = true;
						this->size = 0;
					}
				}
			}
		}

		void generate(const short &size)
		{
			MyRand<T> rd(LOW_BOUND, HIGH_BOUND);
			this->size = size;

			for (unsigned short i = 0; i < MAX_RECORD_LENGTH; ++i)
			{
				if (i < this->size)
				{
					this->numbers[i] = rd();
				}
				else
				{
					if (std::is_same<T, double>::value)
						this->numbers[i] = std::numeric_limits<double>::quiet_NaN();
				}
			}
		}

		bool isValid()
		{
			if (this->size > 0)
				return true;

			return false;
		}

		friend std::ostream& operator<< (std::ostream &out, Record &r)
		{
			for (unsigned short i = 0; i < MAX_RECORD_LENGTH; ++i)
				out << std::hex << r.getNumber(i) << ((i != (MAX_RECORD_LENGTH - 1)) ? " " : "\n");

			return out;
		}

		friend std::istream& operator >> (std::istream &in, Record &r)
		{
			std::string s;

			std::getline(in, s);

			r = Record<T>(s);

			return in;
		}

		T range()
		{
			if (std::isnan(this->numbers[1]))
				return 0;

			T min, max;
			min = this->numbers[0];
			max = min;

			for (unsigned short i = 1; i < this->size; ++i)
			{
				if (std::is_same<T, double>::value && std::isnan(this->numbers[i]))
					break;

				if (min > this->numbers[i])
					min = this->numbers[i];
				else if (max < this->numbers[i])
					max = this->numbers[i];
			}
			return abs(max - min);
		}

		T getNumber(unsigned short &at)
		{
			if (at < MAX_RECORD_LENGTH)
				return this->numbers[at];
			return 0;
		}

	private:
		std::vector<T> numbers;
		unsigned short size;
	};


	template<typename T>
	class Site
	{
	public:
		Site()
		{
			this->records = std::vector<Jereczek::Record<T>>();
			this->records.reserve(SITE_SIZE + 1);
			this->ss.str("");
			this->endOfFile = false;
			this->actualPosition = 0;
			this->siteSaves = 0;
			this->siteLoads = 0;
			this->endOfSerial = false;
		}

		void addRecord(Jereczek::Record<T> &r, bool eof = false)
		{
			this->records.push_back(r);
			ss << r;
			if ((this->records.size() % SITE_SIZE) == 0)
				this->siteLoads++;
			if (eof)
			{
				this->loaded();
			}
		}

		void loaded()
		{
			this->endOfFile = true;
			this->siteLoads++;
		}

		unsigned int getSiteSaves()
		{
			return this->siteSaves;
		}

		unsigned int getSiteLoads()
		{
			return this->siteLoads;
		}

		void clear()
		{
			this->records.clear();
			this->ss.str(std::string());
			this->actualPosition = 0;
			this->endOfFile = false;
		}

		bool isSaveNecessary()
		{
			if (this->records.size() == SITE_SIZE || this->endOfFile)
				return true;

			return false;
		}

		std::string save()
		{
			std::string result;

			result = this->ss.str();
			this->clear();
			this->siteSaves++;

			return result;
		}

		bool isSiteEmpty()
		{
			if (this->actualPosition < this->records.size())
				return false;

			//this->clear();
			return true;
		}

		void setEOF(bool b)
		{
			this->endOfFile = b;
		}

		void clearCounter()
		{
			this->siteLoads = 0;
			this->siteSaves = 0;
		}

		T getRange()
		{
			return this->records.at(this->actualPosition).range();
		}

		//koniec serii
		void setEOS(bool s)
		{
			this->endOfSerial = s;
		}

		bool EOS()
		{
			return this->endOfSerial;
		}

		Jereczek::Record<T> getRecord()
		{
			return this->records.at(this->actualPosition++);
		}

	private:
		unsigned int actualPosition, siteSaves, siteLoads;
		bool endOfFile, endOfSerial;
		std::vector<Jereczek::Record<T>> records;
		std::stringstream ss;
	};


	template<typename T>
	class Tape
	{
	public:
		Tape(const unsigned short &index)
		{
			this->tapeName += std::to_string(index);
			this->tapeName += ".tape";
			this->endOfFile = false;
		}

		Tape(Jereczek::Tape<T> &t)
		{
			this->tapeName = t.tapeName;
			this->endOfFile = t.endOfFile;
		}

		~Tape()
		{

		}

		unsigned getSiteSaves()
		{
			return this->site.getSiteSaves();
		}

		unsigned getSiteLoads()
		{
			return this->site.getSiteLoads();
		}

		void addSerial(std::vector<Jereczek::Record<T>> &s, bool eof = false)
		{
			for (unsigned int i = 0; i < s.size(); ++i)
			{
				if (i < s.size() - 1)
					this->site.addRecord(s.at(i));
				else
					this->site.addRecord(s.at(i), eof);

				if (this->site.isSaveNecessary())
					this->save();
			}
		}

		void addRecord(Jereczek::Record<T> &r, bool eof = false)
		{
			this->site.addRecord(r, eof);

			if (this->site.isSaveNecessary())
				this->save();
		}

		void clear()
		{
			this->site.clear();
		}

		void create()
		{
			this->file.open(this->tapeName, std::ios::out | std::ios::trunc);

			if (this->file.good())
			{
				this->file.close();
			}
			else
			{
				std::cout << "Nie mozna utworzyc tasmy: " << this->tapeName << "\n";
			}
		}

		void save()
		{
			if (this->file.good())
			{
				this->file << this->site.save();
			}
			else
				std::cout << "Nie mozna otworzyc: " << this->tapeName << "\n";
		}

		//type:= true->readOnly | false->writeOnly
		bool open(bool type = false)
		{
			if (type)
				this->file.open(this->tapeName, std::ios::in);
			else
				this->file.open(this->tapeName, std::ios::out | std::ios::trunc);


			if (this->file.good())
			{
				this->endOfFile = true;
				return true;
			}

			std::cout << "Nie mozna otworzyc tasmy: " << this->tapeName << "\n";
			return false;
		}

		T getRange()
		{
			return this->site.getRange();
		}

		//jeœli brak rekordu wczytaj z pliku
		Jereczek::Record<T> getRecord()
		{
			if (this->site.isSiteEmpty())
			{
				this->site.clear();
				this->load();
			}

			return this->site.getRecord();
		}

		void close()
		{
			if (this->file.good())
				this->file.close();

			this->endOfFile = false;
		}

		void load(bool show = false)
		{
			if (this->file.good())
			{
				Jereczek::Record<T> record;
				std::string buffer;
				while (true)
				{
					std::getline(this->file, buffer);

					if (!buffer.empty())
					{
						record = Jereczek::Record<T>(buffer);
						if (record.isValid())
							this->site.addRecord(record);
					}

					if (this->site.isSaveNecessary())
					{
						if (show)
							std::cout << this->site.save();
						else
							//strona jest pe³na
							break;
					}
					else if (buffer.empty())
					{
						if (show)
							std::cout << this->site.save();
						this->endOfFile = false;
						this->file.close();
						break;
					}
				}
			}
			else
				std::cout << "Nie mozna odczytywac z tasmy: " << this->tapeName << "\n";
		}

		//false->aby zobaczyc tyko sam stan strony, potrzebne do doczytywania rekordów przy scalaniu
		bool isEmpty(bool merging = true)
		{
			if (merging)
			{
				if (!this->endOfFile && this->site.isSiteEmpty())
					return true;

				return false;
			}
			else
			{
				if (this->endOfFile && this->site.isSiteEmpty())
					return true;

				return false;
			}
		}

		std::string getName()
		{
			return this->tapeName;
		}

	private:
		bool endOfFile;
		std::string tapeName;
		std::fstream file;
		Jereczek::Site<T> site;
	};


	template<typename T>
	class Phase
	{
	public:
		Phase()
		{
			this->tapes = std::vector<Jereczek::Tape<T>>();
			this->actualTape = 0;
			this->phaseCounter = 0;
			this->tapeChanged = false;
			this->distributionStarted = false;

			for (unsigned short i = 0; i < TAPES_AMOUNT; ++i)
			{
				this->tapes.push_back(Jereczek::Tape<T>(i));
				this->tapes.at(i).create();
			}

			this->helpRanges = std::vector<T>(TAPES_AMOUNT - 1);
		}

		bool setSerial(Jereczek::Record<T> &r)
		{
			bool isRecordRangeBigger = true;

			if (this->serial.empty())
			{
				this->serial.push_back(r);
			}
			else
			{
				if (r.range() >= this->serial[this->serial.size() - 1].range())
				{
					//seria jest pe³na
					if (this->serial.size() > SITE_SIZE)
					{
						this->tapes.at(this->actualTape).addSerial(this->serial);
						this->serial.clear();
						this->setSerial(r);
					}
					else
						this->serial.push_back(r);
				}
				else
					isRecordRangeBigger = false;
			}

			return isRecordRangeBigger;
		}

		//taœmy dystrybucyjne od razu ³aduj¹ zawartoœæ
		bool openTapes(bool merging = true)
		{
			if (merging)
			{

				for (unsigned int i = 0; i < TAPES_AMOUNT - 1; ++i)
				{
					if (!this->tapes.at(i).open(true))
						return false;
					this->tapes.at(i).load();
					this->helpRanges.at(i) = -1.0;
				}

				if (!this->tapes.back().open())
					return false;
				return true;
			}
			else
			{
				for (unsigned int i = 0; i < TAPES_AMOUNT - 1; ++i)
				{
					if (!this->tapes.at(i).open())
						return false;
				}
				return true;
			}
		}

		void actualState()
		{
			////Jereczek::cls():
			this->tapes.back().open(true);
			this->tapes.back().load(true);

			std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
			//std::getline(std::cin, sign);
		}

		void getSortedFile()
		{
			this->tapes.back().load(true);
		}

		int minPosition()
		{
			int position = -1, i, j;
			T min, temp;

			for (i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				if (this->tapes.at(i).isEmpty(false))
				{
					this->tapes.at(i).clear();
					this->tapes.at(i).load();
				}
				if (!this->tapes.at(i).isEmpty())
				{
					temp = this->tapes.at(i).getRange();
					if (temp >= this->helpRanges.at(i))
					{
						this->helpRanges.at(i) = temp;
						min = temp;
						position = i;
						break;
					}
				}
			}

			for (j = i + 1; j < TAPES_AMOUNT - 1; ++j)
			{
				if (this->tapes.at(j).isEmpty(false))
				{
					this->tapes.at(j).clear();
					this->tapes.at(j).load();
				}
				if (!this->tapes.at(j).isEmpty() && (temp = this->tapes.at(j).getRange()) < min)
				{
					if (temp >= this->helpRanges.at(j))
					{
						this->helpRanges.at(j) = temp;
						min = temp;
						position = j;
					}
				}
			}

			return position;
		}

		unsigned getSiteLoads()
		{
			unsigned result = 0;
			for (unsigned i = 0; i < TAPES_AMOUNT; ++i)
				result += this->tapes.at(i).getSiteLoads();

			return result;
		}

		unsigned getSiteSaves()
		{
			unsigned result = 0;
			for (unsigned i = 0; i < TAPES_AMOUNT; ++i)
				result += this->tapes.at(i).getSiteSaves();

			return result;
		}

		bool isNextSerials()
		{
			for (unsigned i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				if (!this->tapes.at(i).EOS())
					return false;
			}
			return true;
		}

		void nextSerials()
		{
			for (unsigned i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				//this->tapes.at(i).setEOS(false);
				this->helpRanges.at(i) = -1.0;
			}
		}

		void merge()
		{
			this->phaseCounter++;

			//otworzenie i wczytanie zawartoœci
			if (this->openTapes())
			{
				int position = -1;
				while (true)
				{
					position = this->minPosition();

					if (position == -1)
						this->nextSerials();

					if (isMergeEnd())
						break;

					if (position != -1)
						this->tapes.back().addRecord(this->tapes.at(position).getRecord());
				}

				this->endOfMerge();
			}
		}

		bool isMergeEnd()
		{
			for (unsigned i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				if (!this->tapes.at(i).isEmpty())
					return false;
			}

			return true;
		}

		//zapis z zamkniêciem pliku
		void endOfMerge()
		{
			for (unsigned i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				this->tapes.at(i).clear();
				this->tapes.at(i).close();
			}
			this->tapes.back().save();
			this->tapes.back().close();
		}

		//return:= true->wykryto tylko jedn¹ seriê posortowan¹ | false->wykryto wiele serii
		bool distribution(Jereczek::Site<T> &s, bool eof = false)
		{
			bool sorted = false;
			if (this->distributionStarted || this->openTapes(false))
			{
				Jereczek::Record<T> r;

				if (!this->distributionStarted)
					this->distributionStarted = true;

				while (!s.isSiteEmpty())
				{
					r = s.getRecord();
					//dodanie rekordu do serii
					if (!this->setSerial(r))
					{
						this->tapes[this->actualTape++].addSerial(this->serial);
						this->serial.clear();
						this->setSerial(r);

						if (!this->tapeChanged)
							tapeChanged = true;

						this->validateActualTape();
					}
				}
				s.clear();

				if (eof)
				{
					//jedna seria posortowana
					if (!this->tapeChanged)
						sorted = true;

					this->tapes[this->actualTape].addSerial(this->serial, eof);
					this->endOfDistribution();

					if (!sorted)
						this->merge();
				}
			}
			return sorted;
		}

		std::string getMergeTapeName()
		{
			return this->tapes.back().getName();
		}

		void endOfDistribution()
		{
			this->serial.clear();
			this->actualTape = 0;
			this->freeTapes();
			this->tapeChanged = false;
			this->distributionStarted = false;
		}

		//zapisz do taœmy to co jest na stronie
		void freeTapes()
		{
			for (unsigned int i = 0; i < TAPES_AMOUNT - 1; ++i)
			{
				this->tapes.at(i).save();
				this->tapes.at(i).close();
			}
		}

		//ustaw aktualn¹ taœmê
		void validateActualTape()
		{
			//schemat 3+1 jedna taœma do scalania
			if (this->actualTape >= TAPES_AMOUNT - 1)
				this->actualTape = 0;
		}

		unsigned int getPhaseCounter()
		{
			return this->phaseCounter;
		}

	private:
		bool tapeChanged, distributionStarted;
		unsigned int actualTape, phaseCounter;
		std::vector<Jereczek::Tape<T>> tapes;
		std::vector<Jereczek::Record<T>> serial;
		std::vector<T> helpRanges;
	};


	template<typename T>
	class Manager
	{
	public:
		Manager()
		{
			this->low = LOW_GENERATED;
			this->high = HIGH_GENERATED;
			this->record = Jereczek::Record<T>();
			this->fileName = "";
			this->site = Jereczek::Site<T>();
			this->phase = Jereczek::Phase<T>();
		}

		void loadSortedFile(bool sorted = false)
		{
			bool quit = false;
			std::string sign;

			while (!quit)
			{
				//Jereczek::cls():
				if (sorted)
					std::cout << "Chcesz podejrzec posortowany plik? (y/n)\n";
				else
					std::cout << "Chcesz podejrzec nieposortowany plik? (y/n)\n";
				std::getline(std::cin, sign);
				if (sign == "y" || sign == "Y")
				{
					////Jereczek::cls():

					this->file.open((!sorted ? this->fileName : this->getMergeTapeName()), std::ios::in);
					if (this->file.good())
					{
						Jereczek::Record<T> record;
						std::string buffer;
						while (true)
						{
							std::getline(this->file, buffer);

							if (!buffer.empty())
							{
								record = Jereczek::Record<T>(buffer);
								if (record.isValid())
									this->site.addRecord(record);
							}

							if (this->site.isSaveNecessary())
							{
								std::cout << this->site.save();
							}
							else if (buffer.empty())
							{
								std::cout << this->site.save();
								this->file.close();
								break;
							}
						}
					}

					quit = true;
					std::getline(std::cin, sign);
				}
				else if (sign == "n" || sign == "N")
				{
					quit = true;
				}
			}
		}

		//sprawdzenie czy plik o danej nazwie mo¿e byæ u¿yty (createFile:= true-stwórz plik || false-otwórz plik)
		void createOrOpenFile(bool createFile)
		{
			bool failed = false;

			while (true)
			{
				this->getFileName(failed);

				//pozwala utworzyæ plik
				if (createFile)
					this->file.open(this->fileName, std::ios::out | std::ios::trunc);
				else
					this->file.open(this->fileName, std::ios::in);

				if (this->file.is_open())
				{
					this->file.close();
					break;
				}
				else
				{
					failed = true;
				}
			}
		}

		void generate()
		{
			MyRand<unsigned int> rd1(low, high);
			MyRand<unsigned int> rd2(MIN_RECORD_LENGTH, MAX_RECORD_LENGTH);

			this->file.open(this->fileName, std::ios::out);

			if (this->file.good())
			{
				unsigned int n = rd1();
				for (unsigned int i = 0; i < n; ++i)
				{
					this->record.generate(rd2());
					//this->file << this->record;
					this->site.addRecord(record, (i == n - 1 ? true : false));
					if (this->site.isSaveNecessary())
					{
						this->file << this->site.save();
					}
				}

				this->file.close();
			}
		}

		unsigned int getSiteLoads()
		{
			return this->site.getSiteLoads() + this->phase.getSiteLoads();
		}

		unsigned int getSiteSaves()
		{
			return this->site.getSiteSaves() + this->phase.getSiteSaves();
		}

		void loadInfo()
		{
			this->loads = this->getSiteLoads();
			this->saves = this->getSiteSaves();
		}

		void displayInfo()
		{
			std::string s;
			//Jereczek::cls():
			std::cout << "Informacje koncowe:\n";
			std::cout << "Liczba faz sortowania | Liczba zapisow stron | Liczba odczytow stron\n";
			std::cout << "\t" << this->phase.getPhaseCounter() << "\t\t\t" << this->saves << "\t\t\t" << this->loads << "\n";
			std::getline(std::cin, s);
		}

		void setBorders()
		{
			std::string sign;
			//Jereczek::cls():
			std::cout << "Chcesz zmienic domyslna wielkosc danych (min: " << LOW_GENERATED << " max: " << HIGH_GENERATED << ")? (y/n)";
			while (true)
			{
				std::getline(std::cin, sign);
				if (sign == "y" || sign == "Y")
				{
					std::cout << "Min:";
					std::getline(std::cin, sign);
					this->low = std::stoi(sign);
					std::cout << "Max:";
					std::getline(std::cin, sign);
					this->high = std::stoi(sign);
					if (low > high)
					{
						std::swap(this->low, this->high);
					}
					break;
				}
				else if (sign == "n" || sign == "N")
				{
					break;
				}
			}
		}

		void loadFromFile(bool displayAfterPhase = false)
		{
			bool isInitDistribution = false;
			bool sorted = false;

			this->site.clearCounter();

			while (!sorted)
			{
				this->file.open((isInitDistribution ? this->getMergeTapeName() : this->fileName), std::ios::in);
				this->site.setEOF(false);

				if (this->file.good())
				{
					std::string buffer;
					while (true)
					{
						std::getline(this->file, buffer);

						if (!buffer.empty())
						{
							this->record = Jereczek::Record<T>(buffer);
							if (this->record.isValid())
								this->site.addRecord(this->record);
						}

						if (this->site.isSaveNecessary())
						{
							if (this->phase.distribution(this->site))
								sorted = true;
						}
						else if (buffer.empty())
						{
							this->file.close();
							this->site.loaded();
							if (this->phase.distribution(this->site, true))
								sorted = true;

							isInitDistribution = true;
							break;
						}
					}
				}
				if (displayAfterPhase && !sorted)
					this->phase.actualState();
			}
		}

		bool b()
		{
			std::string s;
			//Jereczek::cls():
			std::cout << "Chcesz wyswietlac plik po kazdej fazie? (y/n)\n";

			std::getline(std::cin, s);

			if (s == "y" || s == "Y")
				return true;
			return false;
		}

		std::string getMergeTapeName()
		{
			return this->phase.getMergeTapeName();
		}

		void loadFromKeyboard()
		{
			//Jereczek::cls():
			std::cout << "Wczytywanie konczy sie po pierwszej pustej linii!\n";
			std::cout << "Pojedynczy rekord jest jedna linia! Cyfra po cyfrze.\n";
			std::cout << "Maksymalnie 15 cyfr w linii, reszta jest ucinana!\n";

			this->file.open(this->fileName, std::ios::out);
			std::string buffer;

			if (this->file.good())
			{
				while (true)
				{
					std::getline(std::cin, buffer);

					if (!buffer.empty())
					{
						this->record = Jereczek::Record<T>(buffer);
						if (this->record.isValid())
							this->site.addRecord(record);
					}

					if (this->site.isSaveNecessary() || buffer.empty())
						this->file << this->site.save();

					if (buffer.empty())
					{
						this->file.close();
						this->site.loaded();
						break;
					}
				}
			}
		}

		void getFileName(bool flag)
		{
			bool reserved = false;
			std::regex reservedFile("(.*\\.tape)");
			while (true)
			{
				//Jereczek::cls():
				if (reserved)
					std::cout << "Rozszerzenie pliku zarezerwowane!\n";
				else if (flag)
					std::cout << "Nie udalo sie otworzyc pliku!\n";
				std::cout << "Podaj nazwe pliku:\n";
				std::getline(std::cin, this->fileName);

				if (std::regex_match(this->fileName, reservedFile))
					reserved = true;
				else if (!this->fileName.empty())
					break;
			}
		}

	private:
		unsigned int low, high, loads, saves;
		std::string fileName;
		std::fstream file;
		Jereczek::Record<T> record;
		Jereczek::Site<T> site;
		Jereczek::Phase<T> phase;
	};


	class Menu
	{
	public:
		Menu()
		{
			this->selectedOption = 0;
			this->menuSize = 5;
			//Jereczek::cls():
			this->menu = "\t\tSBD - project 1 <Adam Jereczek>\n1. Generuj dane.\n2. Wczytaj dane z pliku.\n3. Wczytaj dane z klawiatury.\n4. Wyjscie\n";
		}

		unsigned short selectOption()
		{
			bool quit = false;
			unsigned short option = 0;
			std::string line;

			while (true)
			{
				this->display(this->menu);
				std::getline(std::cin, line);
				option = atoi(line.c_str());
				if (option > 0 && option < this->menuSize)
				{
					return option;
				}
			}
		}

		void display(const std::string &menu)
		{
			//Jereczek::cls():
			std::cout << menu;
		}

	private:
		unsigned short selectedOption, menuSize;
		std::string menu;
	};
}


int main()
{
	unsigned int selectedOption;
	Jereczek::Menu menu = Jereczek::Menu();

	selectedOption = menu.selectOption();

	Jereczek::Manager<double> man;
	//generuj dane
	if (selectedOption == 1)
	{
		man.createOrOpenFile(true);
		//ustawienie granic min i max granic losowania iloœci rekordów
		man.setBorders();
		man.generate();
	}
	//wczytaj z pliku
	else if (selectedOption == 2)
	{
		man.createOrOpenFile(false);
	}
	//wczytaj z klawiatury
	else if (selectedOption == 3)
	{
		man.createOrOpenFile(true);
		man.loadFromKeyboard();
	}
	//wyjœcie
	else if (selectedOption == 4)
	{
		return 0;
	}
	man.loadSortedFile(false);

	man.loadFromFile(man.b());

	man.loadInfo();

	man.loadSortedFile(true);

	man.displayInfo();


	return 0;
}