#include <threading.h>
#include <ucontext.h> // need it for context switching

//using this helper to fix the -Wclobbered error showing in terminal
static int getcontext_safe(ucontext_t *uc) {
    return getcontext(uc); //calling getcontext and return its result
}

void t_init(void) //initialzing the threads
{
    for (int i = 0; i < NUM_CTX; ++i) {

        //initialzing various data structures that library uses such as contexts and current_context_idx
        contexts[i].state = INVALID; // mark all contexts as invalid

        contexts[i].context.uc_stack.ss_sp = NULL; // clear stack pointer
        contexts[i].context.uc_stack.ss_size = 0; // clear stack size
        contexts[i].context.uc_link = NULL; // no next context

    }

    //initialize the main context
    getcontext(&contexts[0].context); //get the current context
    contexts[0].state = VALID; // mark main context as valid 

    current_context_idx = 0; // set current context to main context
    
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)  // creating a new thread
{

    //need worker context struct to store each threads context and current state
    struct worker_context *w = NULL; // set to NULL initially since we haven't found a slot yet
    {
        // find first available slot
        for (int j = 1; j < NUM_CTX; ++j) { // start from so we skip main context (dont want to overwrie it)
            if (contexts[j].state == INVALID) { //if we find an invalid slot
                w = &contexts[j]; // point w to that slot
                break; //stop searching 
            }
        }
    }

    if (w == NULL) { 
        return -1; // no available slot found after searching all contexts
    }                     

    ucontext_t *uc = &w->context; // geting pointer to context inside the found slot

    // if getcontext fails, return -1 
    if (getcontext_safe(uc) == -1) {
        return -1;
    }

    // allocate stack for new context
    void *stk = malloc(STK_SZ); // allocate memory for stack

    // if malloc fails, return -1
    if (!stk) {
        return -1; 
    }

    uc->uc_stack.ss_sp= stk; // set stack pointer
    uc->uc_stack.ss_size = STK_SZ; // set stack size
    uc->uc_link= NULL; // no next context

    //initialze the context so it begins executing foo)arg1,arg2) when switched to
    makecontext(uc, (void (*)(void))foo, 2, arg1, arg2); 

    w->state = VALID; //mark as ready to be used 
    return 0;
}

int32_t t_yield() //yielding the current thread
{
    int current = current_context_idx; // get current context index of the running thread

    // count how many other contexts are runnable, aka VALID
    int runnable_others = 0; 

    for (int i = 0; i < NUM_CTX; ++i) { //loop through all contexts
        if (i != current && contexts[i].state == VALID) { // if not current and is VALID 
            ++runnable_others; // then increment count by 1
        }
    }

    //if there are no other runnable contexts
    if (runnable_others == 0) {
        return -1;
    }

    //finding the next context that is runnable
    int next = -1; //initialize to -1 so if we find one we can update it

    for (int step = 1; step < NUM_CTX; ++step) { //loop through all to find
        int idx = (current + step) % NUM_CTX; // wrap around using modulo so we stay within the bounds
        if (contexts[idx].state == VALID) { // if we find a valid context
            next = idx; // set next to that index
            break;
        }
    }
    if (next == -1) { // if we didnt find a next context
        return -1; // return -1
    }

    //do the context switching
    int prev = current; // store previous context index
    current_context_idx = (uint8_t)next; //updating current context index to next
    if (swapcontext(&contexts[prev].context, &contexts[next].context) == -1) { //save current context and switch to the next context
        current_context_idx = (uint8_t)prev; //if swapcontext fails, revert back to previous context index
        return -1; // return -1 for failre
    }

    //ruturining the num of other runnable contexts
    return runnable_others;
}


void t_finish(void) //finishing the current thread
{
    int current = current_context_idx; //get current context index

    // free stack if not main context
    if (current != 0 && contexts[current].context.uc_stack.ss_sp != NULL) { 
        free(contexts[current].context.uc_stack.ss_sp); // free the allocated stack memory
        contexts[current].context.uc_stack.ss_sp = NULL; // clear stack pointer
        contexts[current].context.uc_stack.ss_size = 0;  // clear stack size
    }

    contexts[current].state = INVALID; // mark current context as INVALID since its done

    current_context_idx = 0; // switch back to main context
    setcontext(&contexts[0].context); // switch to main context

}
