#ifndef __GAMERUN_H
#define __GAMERUN_H
#define BLANK 0
#define NUM_SPECIES 7
#include "../Part1/Headers.hpp"
#include "../Part2/Thread.hpp"
#include "../Part1/PCQueue.hpp"
#include <cmath>
/*--------------------------------------------------------------------------------
								  Species colors
--------------------------------------------------------------------------------*/
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black - 7 */
#define RED     "\033[31m"      /* Red - 1*/
#define GREEN   "\033[32m"      /* Green - 2*/
#define YELLOW  "\033[33m"      /* Yellow - 3*/
#define BLUE    "\033[34m"      /* Blue - 4*/
#define MAGENTA "\033[35m"      /* Magenta - 5*/
#define CYAN    "\033[36m"      /* Cyan - 6*/


/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on; 
	bool print_on; 
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:
    class TileJob{
    private:
        Game* game;
        int start;
        int end;
    public:
        TileJob(Game* game, int start,int end): game(game), start(start) ,end(end){}
        ~TileJob() = default;
        void perform_job();
    };

	Game(game_params params):m_gen_num (params.n_gen),m_thread_num(params.n_thread),
	                        file_name(params.filename),interactive_on(params.interactive_on),
	                        print_on(params.print_on),phase(1){

    }
	~Game() = default;
	void run(); // Runs the game
	const vector<double> gen_hist() const; // Returns the generation timing histogram
	const vector<double> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const;
    TileJob* getNextJob();
    pthread_mutex_t tasks_lock;
	int tasksLeft;

protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game(); 
	inline void print_board(const char* header);

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<double> m_tile_hist; 	 // Shared Timing history for tiles: First (2 * m_gen_num) cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<double> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	string file_name;
	int field_height;
	int field_width;
    vector<vector<unsigned int>>* curr;
    vector<vector<unsigned int>>* next;
    int phase;
    int jobs_completed;
    pthread_cond_t phase_done;
    pthread_mutex_t lock;
    PCQueue<TileJob*>* jobs_queue;



    bool on_board(int i, int j);

    int count_live_neighbors(int i,int j,int* dominant);

    void update_next_phase1(int i,int j,bool alive, int num_neighbors,int dominant);

    int new_species(int i,int j);

	void phase1(int start, int end);

    void phase2(int start, int end);

    void swap_fields();

	// TODO: Add in your variables and synchronization primitives  

};

class Working_Thread : public Thread {
protected:
    void thread_workload() override;
public:
    Game* game;
    Working_Thread(uint thread_id,Game* game): Thread(thread_id), game(game){}
    virtual ~Working_Thread() = default;

};
#endif
