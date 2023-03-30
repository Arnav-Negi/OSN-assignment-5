## Question - 2: Pizzeria Gino Sorbillo

### How to run : 

- ```Run make in the q2 folder. ```
- ```Run ./q2. ```
- ```Enter input```

### Implementation :
The customers and chefs are run as threads, each with their own structs and thread functions. The ovens are a set of n semaphores.
<br>
The pizzas are stored in a list which can be accessed by the chefs and customers.
<br>
The main thread controls the global time variable for the simulation.
<br>

Chef thread:

- The chef waits till his arrival, then arrives at the restaurant.
-  While at the restaurant, picks a pizza which he can make in time with given ingredients.
-  Waits 3 seconds to prepare for pizza using ingredients.
-  Waits for oven (sem_trywait) and puts pizza in when he gets one.
-  Once pizza is done in oven, waits for customer to give it to him.
- Repeat until he leaves

Customer thread:

- Waits till arrival.
- Places order upon arrival. Order placed in global pizza queue 'ordered_pizzas'.
- Drives till pickup.
- If order rejected, leaves drive thru immediately.
- Waits at pickup untill all his pizzas are either served or rejected.

### Follow-up Questions:

`The pick-up spot now has a stipulated amount of pizzas it can hold. If the
pizzas are full, chefs route the pizzas to a secondary storage. How would you
handle such a situation?`
The pizzas can be stored in two seperate queues, one which can grow indefinitely (storage) which the customers cant directly access, and the other with the limited number of pizzas (pickup spot). The customer threads access the pickup queue to check if their pizzas have arrived, and after taking one, they add a pizza from the storage queue to this queue.

`Each incomplete order affects the ratings of the restaurant. Given the past
histories of orders, how would you re-design your simulation to have lesser
incomplete orders? Note that the rating of the restaurant is not affected if the order is rejected instantaneously on arrival.`
While placing the order, if even one of the customer's cant be made, then the customer is rejected. This makes it so only complete orders are given out to the customers and ratings dont get affected.

`Ingredients can be replenished on calling the nearest supermarket. How
would your drive-thru rejection / acceptance change based on this?`
Customers wont have to be rejected unless no chefs can make the pizza in time. Hence a chef will be able to wait for ingredients to replenish in the case there arent enough.