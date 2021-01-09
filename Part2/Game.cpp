#include "../Part2/Game.hpp"
#include "../Part2/utils.hpp"
#include "../Part2/Thread.hpp"

static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/

void Game::run() {
	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation
        auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(nullptr);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	// Create game fields - Consider using utils:read_file, utils::split
    vector<string> all_lines = utils::read_lines(file_name);
	field_height = all_lines.size() ;
    pthread_mutex_init(&tasks_lock, nullptr);
    pthread_cond_init(&phase_done, nullptr);
	pthread_mutex_init(&lock, nullptr);
    curr = new vector<vector<unsigned int>> ;
    next = new vector<vector<unsigned int>> ;
    jobs_queue = new PCQueue<TileJob*>;


    vector<string> cur_line;

    // fill curr field with input, next remains zeros (will be updated in first move)
    for (int i=0 ;i<field_height;i++) {
        cur_line = utils::split(all_lines[i], ' ');
        if (i == 0){
            field_width = cur_line.size();
        }
        vector<unsigned int>* cur_line_vals = new vector<unsigned int>;
        for(int j=0;j<field_width;j++){
            unsigned int num = std::stoi(cur_line[j]);
            cur_line_vals->push_back(num);
        }
        curr->push_back(*cur_line_vals);
        next->push_back(*cur_line_vals);
    }


	// Create & Start threads
	m_thread_num = thread_num();
    tasksLeft = 2*m_thread_num*m_gen_num;
    for (int i=0 ; i<m_thread_num ; i++){
        Working_Thread* thread = new Working_Thread(i,this);
	    thread->start();
	    m_threadpool.push_back(thread);
	}
	// Testing of your implementation will presume all threads are started here
}


void Game::_step(uint curr_gen) {
    int tile_size = field_height/m_thread_num;
    jobs_completed = 0;
//    int last_tile_size = tile_size + field_height%m_thread_num;
//phase 1 jobs
    // Push jobs to queue

    for ( int i=0 ; i < m_thread_num ; i ++ ){
        if (i == m_thread_num - 1){
            TileJob* job = new TileJob(this,i*tile_size,field_height-1);
            jobs_queue->push(job);
        }
        else{
            TileJob* job = new TileJob(this,i*tile_size,(i+1)*tile_size-1);

            jobs_queue->push(job);
        }
    }
    // Wait for the workers to finish calculating
	pthread_mutex_lock(&lock);

    while (jobs_completed < m_thread_num){ //phase not completed
        pthread_cond_wait(&phase_done,&lock);
    }
    phase = 2;
    jobs_completed = 0;
    pthread_mutex_unlock(&lock);
	// Swap pointers between current and next field
	swap_fields();
	// NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade

	//phase 2 jobs
    for ( int i=0 ; i < m_thread_num ; i ++ ){
        if (i == m_thread_num - 1){
            TileJob* job = new TileJob(this,i*tile_size,field_height-1);
            jobs_queue->push(job);
        }
        else{
            TileJob* job = new TileJob(this,i*tile_size,(i+1)*tile_size-1);
            jobs_queue->push(job);
        }
    }

    // Wait for the workers to finish calculating
    pthread_mutex_lock(&lock);
    while (jobs_completed < m_thread_num){ //phase not completed
        pthread_cond_wait(&phase_done,&lock);
    }
    phase = 1;
    jobs_completed = 0;
    pthread_mutex_unlock(&lock);
    // Swap pointers between current and next field
    swap_fields();
    // NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade
}


void Game::_destroy_game() {
    // Destroys board and frees all threads and resources
    // Not implemented in the Game's destructor for testing purposes.
    // All threads must be joined here


    for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }
    delete curr;
    delete next;
    delete jobs_queue;
}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;

        cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
        for (uint i = 0; i < field_height; ++i) {
            cout << u8"║";
            for (uint j = 0; j < field_width; ++j) {
                if ((*curr)[i][j] > 0)
                    cout << colors[(*curr)[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
            }
            cout << u8"║" << endl;
        }
        cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

const vector<double> Game::gen_hist() const {
    return m_gen_hist;
}
const vector<double> Game::tile_hist() const {
    return m_tile_hist;
}

bool Game::on_board(int i, int j) {
    return (i >= 0 && i < field_height && j >= 0 && j < field_width);
}

int Game::count_live_neighbors(int i, int j, int *dominant) {
    vector<unsigned int> species_count (NUM_SPECIES+1,0);
    int neighbors = 0;
    for (int x = i-1 ; x <= i+1 ; x++){
        for (int y = j-1 ; y <= j+1 ; y++){
            if (x == i && y==j) //self
                continue;
            if (on_board(x,y) && (*curr)[x][y] > 0 ) {
                neighbors++;
                if (neighbors <=3){
                    species_count[(*curr)[x][y]] ++;
                }
                if (neighbors == 3) { // get dominant
                    int max_species = 0;
                    int max_count = 0;
                    for (int species = 1; species <= NUM_SPECIES;species ++){
                        if ((species * (species_count)[species]) > max_count){
                            max_count = species * species_count[species];
                            max_species = species;
                        }
                        *dominant = max_species;
                    }
                }
            }
        }
    }
    return neighbors;
}

void Game::update_next_phase1(int i, int j, bool alive, int num_neighbors, int dominant) {
    if (alive && (num_neighbors == 2 || num_neighbors == 3)) // stay the same
        (*next)[i][j] = (*curr)[i][j];
    else if ((!alive) && num_neighbors == 3){ // birth
        (*next)[i][j] = dominant;
    }
    else{
        (*next)[i][j] = 0;
    }
}

int Game::new_species(int i, int j) {
    int sum = 0;
    int num_alive = 0;
    for (int x = i-1 ; x <= i+1 ; x++) {
        for (int y = j - 1; y <= j + 1; y++) {
            if (on_board(x,y) && (*curr)[x][y] != 0){
                num_alive ++;
                sum += (*curr)[x][y];
            }
        }
    }
    return round(double(sum)/num_alive);
}

void Game::phase1(int start, int end) {


    for (int i=start; i<=end;i++){
        for (int j=0; j<field_width; j++){
            bool alive = ((*curr)[i][j] != 0) ;
            int dominant = 0; // only relevant for case 1 - 3 neighbors, was dead
            int num_neighbors = count_live_neighbors(i,j,&dominant);
            update_next_phase1(i,j,alive,num_neighbors,dominant);
        }
    }
}

void Game::phase2(int start, int end) {
    for (int i = start; i <= end; i++) {
        for (int j = 0; j < field_width; j++) {
            if ((*curr)[i][j] != 0)
                (*next)[i][j] = new_species(i,j);
            else
                (*next)[i][j] = 0;
        }
    }
}

void Game::swap_fields() {
    vector<vector<unsigned int>>* temp;
    temp = curr;
    curr = next;
    next = temp;
}

uint Game::thread_num() const {//Returns the effective number of running threads = min(thread_num, field_height)
    return fmin(m_thread_num,field_height);
}

Game::TileJob *Game::getNextJob() {
    return jobs_queue->pop();
}


void Working_Thread::thread_workload() {
    pthread_mutex_lock(&game->tasks_lock);
    while (game->tasksLeft){
        Game::TileJob *job = game->getNextJob();
        game->tasksLeft--;
        job->perform_job();
        delete job;
    }
    pthread_mutex_unlock(&game->tasks_lock);
    return;
}

void Game::TileJob::perform_job() {
    auto tile_start = std::chrono::system_clock::now();

    if (game->phase == 1){
        game->phase1(start,end);
    }
    else {
        game->phase2(start,end);
    }
    //lock, increase jobs, signal next phase waiters if done, add timer all together
    pthread_mutex_lock(&game->lock);
    game->jobs_completed++;
    if (game->jobs_completed == game->m_thread_num)
        pthread_cond_signal(&game->phase_done);
    auto tile_end = std::chrono::system_clock::now();
    game->m_tile_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count());
    pthread_mutex_unlock(&game->lock);
}
