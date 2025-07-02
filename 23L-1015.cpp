#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>

using namespace std;

class Card {
public:
    int rank;
    char suit;
    bool isFaceUp;

    // Default constructor
    Card() {
        rank = 0;
        suit = '0';
        isFaceUp = false;  
    }

    // Parameterized constructor
    Card(int r, char s, bool faceUp = false) {
        rank = r;
        suit = s;
        isFaceUp = faceUp;
    }

    // Print card details, showing face-down cards as "X" if it's face-down
    void printCard() const {
        if (isFaceUp) {
            cout << suit << " " << rank << endl;
        }
        else {
            cout << "X" << endl;
        }
    }
};

class Node {
public:
    Card val;
    Node* next;
    Node* prev;

    // Constructor for Node
    Node(Card cardVal = Card(), Node* next = nullptr, Node* prev = nullptr)
    {
        this->val = cardVal;
        this->next = next;
        this->prev = prev;
    }
};

struct Move {
    enum MoveType {
        DrawStockToWaste,
        MoveTableauToTableau,
        MoveWasteToTableau,
        MoveWasteToFoundation,
        MoveTableauToFoundation,
        MoveFoundationToTableau,
        ResetStockFromWaste
    };

    MoveType moveType;

    // Common data
    int srcColumn;
    int destColumn;
    int foundationIndex;
    int numOfCards;

    // For storing a moved card
    Card movedCard;

    // For tracking if a card was flipped after the move
    bool flippedCard;
    int flippedColumn;
};

class MoveStack {
private:
    struct MoveNode {
        Move val;
        MoveNode* next;
    };
    MoveNode* top;
    int size;
public:
    MoveStack() : top(nullptr), size(0) {}

    void pushMove(const Move& move) {
        MoveNode* newNode = new MoveNode{ move, top };
        top = newNode;
        size++;
    }

    Move popMove() {
        if (isempty()) {
            throw std::runtime_error("MoveStack is empty");
        }
        MoveNode* node = top;
        Move move = node->val;
        top = top->next;
        delete node;
        size--;
        return move;
    }

    bool isempty() const {
        return size == 0;
    }

    int getsize() const {
        return size;
    }

    ~MoveStack() {
        while (top != nullptr) {
            MoveNode* temp = top;
            top = top->next;
            delete temp;
        }
    }
};

// Only allow moving a King (rank 13)
bool canMoveToEmptyTableau(const Card& card) {
    return card.rank == 13;
}

// check if suit colors are opposite
bool isOppositeColor(char suit1, char suit2) {
    if ((suit1 == 'H' || suit1 == 'D') && (suit2 == 'S' || suit2 == 'C')) {
        return true;
    }
    if ((suit1 == 'S' || suit1 == 'C') && (suit2 == 'H' || suit2 == 'D')) {
        return true;
    }
    return false;
}

// Function to check if one card's rank is exactly one less than another
bool isonesmaller(Card card1, Card card2) {
    return (card1.rank == card2.rank - 1);
}

// Doubly Linked List for the tableau columns
class doublylinkedlist {
private:
    Node* head;
    Node* tail;
    int size;

public:
    //default constructor
    doublylinkedlist() {
        this->head = nullptr;
        this->tail = nullptr;
        size = 0;
    }

    // Getter for head Node
    Node* getHead() const {
        return head;
    }

    // to add card to end of tableau/column
    void addCardToEnd(Card cardVal) {
        Node* newNode = new Node(cardVal);
        addNodeToEnd(newNode);
    }

    // Getter for size
    int getSize() const {
        return size;
    }

    // Print the cards in the list
    void print() const {
        Node* current = head;
        while (current != nullptr) {
            current->val.printCard();
            current = current->next;
        }
        cout << endl;
    }

    // Get Node at a specific index
    Node* getNodeAt(int index) const {
        if (index < 0 || index >= size) {
            return nullptr;
        }
        Node* current = head;
        for (int i = 0; i < index; ++i) {
            current = current->next;
        }
        return current;
    }

    // Swap the values of Nodes at positions i and j
    void swapNodes(int i, int j) {
        if (i == j || i < 0 || j < 0 || i >= size || j >= size) {
            return;
        }

        Node* NodeI = getNodeAt(i);
        Node* NodeJ = getNodeAt(j);

        if (NodeI == nullptr || NodeJ == nullptr) {
            return;
        }

        Card temp = NodeI->val;
        NodeI->val = NodeJ->val;
        NodeJ->val = temp;
    }

    // Add a card to the end of the list
    void addNodeToEnd(Node* cardNode) {
        if (cardNode == nullptr) return;
        cardNode->next = nullptr;
        if (size != 0) {
            this->tail->next = cardNode;
            cardNode->prev = this->tail;
            this->tail = cardNode;
        }
        else {
            this->head = this->tail = cardNode;
            cardNode->prev = nullptr;
        }
        size++;
    }

    // Check if the list is empty
    bool isempty()
    {
        return size == 0;
    }

    // Remove the last card from the list
    Node* removeLastNode() {
        if (size == 0) return nullptr;

        Node* lastNode = this->tail;
        if (this->tail->prev) {
            this->tail = this->tail->prev;
            this->tail->next = nullptr;
        }
        else {
            this->head = this->tail = nullptr;
        }
        size--;
        lastNode->prev = nullptr;
        return lastNode;
    }

    // Add a card to the start of the list
    void addtostart(Card c)
    {
        Node* newNode = new Node(c);
        if (size == 0)
        {
            this->head = this->tail = newNode;
        }
        else {
            newNode->next = this->head;
            this->head->prev = newNode;
            this->head = newNode;
        }
        size++;
    }

    // Move cards from this list to the destination list
    void movecard(doublylinkedlist& destination, int numofcards) {
        if (numofcards <= 0 || this->size == 0) return;

        // Check if numofcards exceeds the size of the source list
        if (numofcards > this->size) numofcards = this->size;

        Node* currentAtEnd = this->tail;

        // Traverse to find the starting Node of the sublist to be moved
        for (int i = 1; i < numofcards && currentAtEnd->prev; i++) {
            currentAtEnd = currentAtEnd->prev;
        }

        // Adjust pointers to detach the sublist from this list
        Node* sourceTail = this->tail;
        this->tail = currentAtEnd->prev;
        if (this->tail) {
            this->tail->next = nullptr;
        }
        else {
            this->head = nullptr; // If the entire list is moved, the source becomes empty
        }

        // Adjust destination list to append the sublist
        if (destination.size > 0) {
            destination.tail->next = currentAtEnd;
            currentAtEnd->prev = destination.tail;
            destination.tail = sourceTail;
        }
        else {
            destination.head = currentAtEnd;
            currentAtEnd->prev = nullptr;
            destination.tail = sourceTail;
        }

        // Update sizes of both lists
        this->size -= numofcards;
        destination.size += numofcards;
    }

    // return size of column
    int getsize() const
    {
        return size;
    }

    // Destructor to clean up memory
    ~doublylinkedlist() {
        Node* current = this->head;
        while (current != nullptr) {
            Node* temp = current->next;
            delete current;
            current = temp;
        }
        this->head = this->tail = nullptr;
        this->size = 0;
    }
};

// Stack class for foundations, stockpile, and wastepile
class stack {
private:

    Node* top;
    int size;

public:
    // default constructor
    stack() {
        top = nullptr;
        size = 0;
    }
    // push a node to the stack
    void pushNode(Node* cardNode) {
        if (cardNode == nullptr) return;
        cardNode->next = top;
        top = cardNode;
        size++;
    }

    // Get the top item of the stack
    Node* getTopNode() const {
        return top;
    }

    // Pop a Node from the stack
    Node* popNode() {
        if (top == nullptr) return nullptr;

        Node* NodeToMove = top;
        top = top->next;
        NodeToMove->next = nullptr;
        size--;
        return NodeToMove;
    }

    // returns the value of top item
    Card topItem() const {
        if (top != nullptr)
            return this->top->val;
        else
            throw std::runtime_error("Stack is empty");
    }

    // Check if the stack is empty
    bool isempty() const {
        return size == 0;
    }

    // Return size of stack
    int getsize() const {
        return size;
    }

    // Destructor to clean up memory
    ~stack() {
        Node* current = this->top;
        while (current != nullptr) {
            Node* temp = current->next;
            delete current;
            current = temp;
        }
        this->top = nullptr;
        this->size = 0;
    }
};

// Game class to manage the overall game logic
class game {
    doublylinkedlist tableau[7];   // Array of 7 linked lists representing tableau columns
    stack foundation[4];           // Array of 4 stacks representing the foundation piles
    stack stockpile;               // Stack for the stockpile
    stack wastepile;               // Stack for the wastepile
    MoveStack commandStack;            // Stack to store commands for undo operations

public:
    // Constructor to initialize and start the game
    game() {
        initializeDeck();
    }

    void initializeDeck() {
        doublylinkedlist deck;
        char suits[] = { 'H', 'S', 'C', 'D' };
        for (char suit : suits) {
            for (int rank = 1; rank <= 13; ++rank) {
                deck.addCardToEnd(Card(rank, suit));
            }
        }

        shuffleDeck(deck);
        dealCards(deck);
    }

    // Flips the next face-down card at the top of the specified tableau column, if it exists.
    void flipNextFaceDownCard(int column) {
        if (!tableau[column].isempty()) {
            Node* newTop = tableau[column].getNodeAt(tableau[column].getsize() - 1);  // Get the top card
            if (newTop != nullptr && !newTop->val.isFaceUp) {  // Check if it's face-down
                newTop->val.isFaceUp = true;  // Flip the card face-up
                cout << "The next card in tableau column " << column + 1 << " is now face-up." << endl;
            }
        }
    }

    void shuffleDeck(doublylinkedlist& deck) {
        srand(unsigned(time(0)));  // Seed random number generation

        int deckSize = deck.getSize();  // Get the size of the deck (52 cards)
        for (int i = 0; i < deckSize - 1; ++i) {
            int j = i + rand() % (deckSize - i);  // Random remaining position
            deck.swapNodes(i, j);  // Swap cards at positions i and j
        }
    }

    void dealCards(doublylinkedlist& deck) {
        Node* current = deck.getHead();

        // Deal cards to the tableau columns
        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j <= i; ++j) {
                if (current != nullptr) {
                    current->val.isFaceUp = (j == i);
                    tableau[i].addNodeToEnd(new Node(current->val));
                    current = current->next;
                }
            }
        }

        // Remaining cards go to the stockpile
        while (current != nullptr) {
            stockpile.pushNode(new Node(current->val));  // Add cards to stockpile
            current = current->next;
        }
    }

    // Move a card(s) from one tableau column to another
    void moveCard(int srcColumn, int destColumn, int numOfCards) {
        // Validate that the source and destination columns are within bounds
        if (srcColumn < 0 || srcColumn >= 7 || destColumn < 0 || destColumn >= 7) {
            cout << "INVALID COLUMN INDICES." << endl;
            return;
        }

        // Count the number of face-up cards in the source column
        int faceUpCount = 0;
        Node* current = tableau[srcColumn].getHead();
        while (current != nullptr) {
            if (current->val.isFaceUp) {
                faceUpCount++;
            }
            else {
                faceUpCount = 0;  // Reset count if a face-down card is encountered
            }
            current = current->next;
        }

        // Check if the user is trying to move more cards than are face-up
        if (numOfCards > faceUpCount) {
            cout << "ERROR: YOU ARE TRYING TO MOVE MORE CARDS THAN ARE FACE-UP. ONLY "
                << faceUpCount << " FACE-UP CARDS AVAILABLE TO MOVE." << endl;
            return;
        }

        // Validate that the source column has enough cards
        if (tableau[srcColumn].getsize() < numOfCards) {
            cout << "NOT ENOUGH CARDS IN THE SOURCE COLUMN." << endl;
            return;
        }

        // Get the first card to be moved
        Node* firstCardNode = tableau[srcColumn].getNodeAt(tableau[srcColumn].getsize() - numOfCards);
        Card firstCardToMove = firstCardNode->val;

        // Validate that the sequence of cards being moved is valid
        current = firstCardNode;
        for (int i = 0; i < numOfCards - 1; ++i) {
            Card currentCard = current->val;
            Card nextCard = current->next->val;

            // Check if the cards are in descending order and alternating colors
            if (!isonesmaller(nextCard, currentCard) || !isOppositeColor(nextCard.suit, currentCard.suit)) {
                cout << "INVALID MOVE: CARDS MUST BE IN DESCENDING ORDER AND ALTERNATING COLORS." << endl;
                return;
            }

            current = current->next;
        }

        // Get the top card of the destination column (if any)
        if (!tableau[destColumn].isempty()) {
            Card topDestCard = tableau[destColumn].getNodeAt(tableau[destColumn].getsize() - 1)->val;

            // Check that the first card being moved is one rank smaller and of the opposite color
            if (!isonesmaller(firstCardToMove, topDestCard) || !isOppositeColor(firstCardToMove.suit, topDestCard.suit)) {
                cout << "INVALID MOVE: THE FIRST CARD MUST BE ONE RANK LOWER THAN THE DESTINATION CARD AND OF THE OPPOSITE COLOR." << endl;
                return;
            }
        }
        else {
            // If the destination column is empty, only allow moving a King
            if (!canMoveToEmptyTableau(firstCardToMove)) {
                cout << "INVALID MOVE: ONLY A KING CAN BE PLACED IN AN EMPTY COLUMN." << endl;
                return;
            }
        }

        // Perform the move
        tableau[srcColumn].movecard(tableau[destColumn], numOfCards);

        // Flip the next face-down card in the source column, if any
        bool flippedCard = false;
        Node* newTop = tableau[srcColumn].getNodeAt(tableau[srcColumn].getsize() - 1);
        if (newTop != nullptr && !newTop->val.isFaceUp) {
            newTop->val.isFaceUp = true;
            flippedCard = true;
        }

        // Record the move
        Move move;
        move.moveType = Move::MoveTableauToTableau;
        move.srcColumn = srcColumn;
        move.destColumn = destColumn;
        move.numOfCards = numOfCards;
        move.flippedCard = flippedCard;
        move.flippedColumn = srcColumn;

        commandStack.pushMove(move);

        cout << "MOVE SUCCESSFUL!" << endl;
    }

    // Puts cards from wastepile to stockpile when stockpile gets empty
    void resetStockpileFromWastepile() {
        stack tempStack;

        // Reverse the order by pushing wastepile cards into a temporary stack
        while (!wastepile.isempty()) {
            tempStack.pushNode(wastepile.popNode());
        }

        // Move cards back from the temporary stack to stockpile
        while (!tempStack.isempty()) {
            stockpile.pushNode(tempStack.popNode());
        }

        cout << "WASTEPILE HAS BEEN RESET INTO THE STOCKPILE." << endl;
    }

    // Draw a card from stockpile
    void drawCardFromStockpile() {
        if (stockpile.isempty()) {
            // Reset the stockpile from the wastepile
            resetStockpileFromWastepile();

            // Record the move
            Move move;
            move.moveType = Move::ResetStockFromWaste;
            commandStack.pushMove(move);
        }
        else {
            // Draw card from stockpile
            Node* cardNode = stockpile.popNode();
            wastepile.pushNode(cardNode);

            // Record the move
            Move move;
            move.moveType = Move::DrawStockToWaste;
            move.movedCard = cardNode->val;

            commandStack.pushMove(move);
        }
    }

    // Move a card from waste to tableau
    void moveFromWasteToTableau(int destColumn) {
        // Check if wastepile is empty
        if (wastepile.isempty()) {
            cout << "WASTEPILE IS EMPTY" << endl;
            return;
        }

        // Pop the top card from the wastepile
        Node* cardNode = wastepile.popNode();
        cardNode->val.isFaceUp = true;

        // Case 1: Destination tableau column is empty
        if (tableau[destColumn].isempty()) {
            if (canMoveToEmptyTableau(cardNode->val)) {
                tableau[destColumn].addNodeToEnd(cardNode);
                cout << "KING MOVED TO EMPTY TABLEAU COLUMN." << endl;
                Move move;
                move.moveType = Move::MoveWasteToTableau;
                move.destColumn = destColumn;
                move.movedCard = cardNode->val;

                commandStack.pushMove(move);
            }
            else {
                // Invalid move, push the card back to the wastepile
                cout << "INVALID MOVE: ONLY A KING CAN BE PLACED IN AN EMPTY TABLEAU COLUMN." << endl;
                wastepile.pushNode(cardNode);
            }
            return;
        }

        // Case 2: Destination tableau column is not empty
        Card lastTableauCard = tableau[destColumn].getNodeAt(tableau[destColumn].getsize() - 1)->val;

        // Check if the card from waste can be moved based on rank and color rules
        if (isOppositeColor(cardNode->val.suit, lastTableauCard.suit) && isonesmaller(cardNode->val, lastTableauCard)) {
            tableau[destColumn].addNodeToEnd(cardNode);  // Move the Node directly to the tableau
            cout << "CARD SUCCESSFULLY MOVED TO TABLEAU." << endl;
            Move move;
            move.moveType = Move::MoveWasteToTableau;
            move.destColumn = destColumn;
            move.movedCard = cardNode->val;

            commandStack.pushMove(move);
        }
        else {
            // Invalid move, push the card back to the wastepile
            cout << "INVALID MOVE: CARD MUST BE OF A DIFFERENT COLOR AND ONE RANK LOWER." << endl;
            wastepile.pushNode(cardNode);
        }
    }

    // Move a card from waste to foundation
    void moveFromWasteToFoundation(int f) {
        // Checks if wastepile is empty
        if (wastepile.isempty()) {
            cout << "WASTEPILE IS EMPTY" << endl;
            return;
        }

        // Checks if the entered foundation number is within range
        if (f < 0 || f > 3) {
            cout << "INVALID FOUNDATION COLUMN. PLEASE USE COLUMNS 0 TO 3." << endl;
            return;
        }

        // Pop the Node from the wastepile (move operation)
        Node* cardNode = wastepile.popNode();

        bool moveSuccessful = false;

        if (foundation[f].isempty()) {
            if (cardNode->val.rank == 1) {
                foundation[f].pushNode(cardNode);
                cout << "CARD MOVED TO EMPTY FOUNDATION PILE." << endl;
                moveSuccessful = true;
            }
            else {
                cout << "ONLY AN ACE CAN BE PLACED IN AN EMPTY FOUNDATION PILE." << endl;
                wastepile.pushNode(cardNode);
            }
        }
        else {
            Card topFoundationCard = foundation[f].topItem();

            if (cardNode->val.suit == topFoundationCard.suit && cardNode->val.rank == topFoundationCard.rank + 1) {
                foundation[f].pushNode(cardNode);
                cout << "CARD SUCCESSFULLY MOVED TO FOUNDATION." << endl;
                moveSuccessful = true;
            }
            else {
                cout << "INVALID MOVE: CARD MUST BE OF THE SAME SUIT AND ONE RANK HIGHER." << endl;
                wastepile.pushNode(cardNode);
            }
        }

        if (moveSuccessful) {
            // Record the move
            Move move;
            move.moveType = Move::MoveWasteToFoundation;
            move.foundationIndex = f;
            move.movedCard = cardNode->val;

            commandStack.pushMove(move);
        }
    }

    // Move a card from tableau to foundation
    void moveFromTableauToFoundation(int srcColumn, int foundationIndex) {
        // Checks if the entered source column is within range and also checks if the entered foundation index is within range
        if (srcColumn < 0 || srcColumn >= 7 || foundationIndex < 0 || foundationIndex >= 4) {
            cout << "INVALID COLUMN OR FOUNDATION INDEX." << endl;
            return;
        }

        // Checks if the tableau is empty
        if (tableau[srcColumn].isempty()) {
            cout << "NO CARDS IN THE TABLEAU COLUMN." << endl;
            return;
        }

        // Pop the Node from tableau
        Node* cardNode = tableau[srcColumn].removeLastNode();

        bool moveSuccessful = false;

        if (foundation[foundationIndex].isempty()) {
            if (cardNode->val.rank == 1) {
                foundation[foundationIndex].pushNode(cardNode);  // Move the Node directly to foundation
                moveSuccessful = true;
                cout << "CARD MOVED TO FOUNDATION." << endl;
            }
            else {
                cout << "ONLY AN ACE CAN BE PLACED IN AN EMPTY FOUNDATION." << endl;
                tableau[srcColumn].addNodeToEnd(cardNode);  // Put the Node back if invalid move
            }
        }
        else {
            Card topFoundationCard = foundation[foundationIndex].topItem();

            if (cardNode->val.suit == topFoundationCard.suit && cardNode->val.rank == topFoundationCard.rank + 1) {
                foundation[foundationIndex].pushNode(cardNode);  // Move the Node to foundation
                moveSuccessful = true;
                cout << "CARD MOVED TO FOUNDATION." << endl;
            }
            else {
                cout << "INVALID MOVE: CARD MUST BE OF THE SAME SUIT AND ONE RANK HIGHER." << endl;
                tableau[srcColumn].addNodeToEnd(cardNode);  // Put the Node back if the move is invalid
            }
        }

        if (moveSuccessful) {
            // Flip the next face-down card (if any)
            bool flippedCard = false;
            Node* newTop = tableau[srcColumn].getNodeAt(tableau[srcColumn].getsize() - 1);
            if (newTop != nullptr && !newTop->val.isFaceUp) {
                newTop->val.isFaceUp = true;
                flippedCard = true;
            }

            // Record the move
            Move move;
            move.moveType = Move::MoveTableauToFoundation;
            move.srcColumn = srcColumn;
            move.foundationIndex = foundationIndex;
            move.movedCard = cardNode->val;
            move.flippedCard = flippedCard;
            move.flippedColumn = srcColumn;

            commandStack.pushMove(move);
        }
    }

    // Move a card from foundation to tableau
    void moveFromFoundationToTableau(int foundationIndex, int destColumn) {
        if (foundationIndex < 0 || foundationIndex >= 4 || destColumn < 0 || destColumn >= 7) {
            cout << "INVALID FOUNDATION OR TABLEAU COLUMN INDEX." << endl;
            return;
        }

        if (foundation[foundationIndex].isempty()) {
            cout << "NO CARDS IN THE FOUNDATION PILE." << endl;
            return;
        }

        // Pop the Node from foundation
        Node* cardNode = foundation[foundationIndex].popNode();

        bool moveSuccessful = false;

        if (tableau[destColumn].isempty()) {
            if (cardNode->val.rank == 13) {  // Only a King can be placed in an empty tableau column
                tableau[destColumn].addNodeToEnd(cardNode);
                moveSuccessful = true;
                cout << "CARD MOVED FROM FOUNDATION TO TABLEAU." << endl;
            }
            else {
                cout << "ONLY A KING CAN BE PLACED IN AN EMPTY TABLEAU COLUMN." << endl;
                foundation[foundationIndex].pushNode(cardNode);  // Put it back if invalid move
            }
        }
        else {
            Card topTableauCard = tableau[destColumn].getNodeAt(tableau[destColumn].getsize() - 1)->val;

            if (isonesmaller(cardNode->val, topTableauCard) && isOppositeColor(cardNode->val.suit, topTableauCard.suit)) {
                tableau[destColumn].addNodeToEnd(cardNode);  // Move the Node to tableau
                moveSuccessful = true;
                cout << "CARD MOVED FROM FOUNDATION TO TABLEAU." << endl;
            }
            else {
                cout << "INVALID MOVE: CARD MUST BE ONE RANK LOWER AND OF OPPOSITE COLOR." << endl;
                foundation[foundationIndex].pushNode(cardNode);  // Put the Node back if the move is invalid
            }
        }

        if (moveSuccessful) {
            // Record the move
            Move move;
            move.moveType = Move::MoveFoundationToTableau;
            move.foundationIndex = foundationIndex;
            move.destColumn = destColumn;
            move.movedCard = cardNode->val;

            commandStack.pushMove(move);
        }
    }

    // Undo the previous move
    void undoMove() {
        if (commandStack.isempty()) {
            cout << "NO MOVES TO UNDO." << endl;
            return;
        }
        Move move = commandStack.popMove();
        switch (move.moveType) {
        case Move::MoveTableauToTableau: {
            // Flip the card back if it was flipped
            if (move.flippedCard) {
                Node* topCardNode = tableau[move.srcColumn].getNodeAt(tableau[move.srcColumn].getsize() - 1);
                if (topCardNode != nullptr && topCardNode->val.isFaceUp) {
                    topCardNode->val.isFaceUp = false;
                }
            }
            // Move cards back from destColumn to srcColumn
            tableau[move.destColumn].movecard(tableau[move.srcColumn], move.numOfCards);
            cout << "UNDO SUCCESSFUL: MOVED CARDS BACK FROM COLUMN " << move.destColumn + 1
                << " TO COLUMN " << move.srcColumn + 1 << "." << endl;
            break;
        }

        case Move::DrawStockToWaste: {
            // Move card back from wastepile to stockpile
            if (!wastepile.isempty()) {
                Node* cardNode = wastepile.popNode();
                stockpile.pushNode(cardNode);
                cout << "UNDO SUCCESSFUL: MOVED CARD BACK FROM WASTEPILE TO STOCKPILE." << endl;
            }
            else {
                cout << "ERROR: WASTEPILE IS EMPTY DURING UNDO." << endl;
            }
            break;
        }

        case Move::MoveWasteToTableau: {
            // Move card back from tableau to wastepile
            if (!tableau[move.destColumn].isempty()) {
                Node* cardNode = tableau[move.destColumn].removeLastNode();
                wastepile.pushNode(cardNode);
                cout << "UNDO SUCCESSFUL: MOVED CARD BACK FROM TABLEAU TO WASTEPILE." << endl;
            }
            else {
                cout << "ERROR: TABLEAU COLUMN IS EMPTY DURING UNDO." << endl;
            }
            break;
        }

        case Move::MoveTableauToFoundation: {
            // Move card back from foundation to tableau
            if (move.flippedCard) {
                Node* topCardNode = tableau[move.srcColumn].getNodeAt(tableau[move.srcColumn].getsize() - 1);
                if (topCardNode != nullptr && topCardNode->val.isFaceUp) {
                    topCardNode->val.isFaceUp = false;
                }
            }
            // Move card back from foundation to tableau
            if (!foundation[move.foundationIndex].isempty()) {
                Node* cardNode = foundation[move.foundationIndex].popNode();
                tableau[move.srcColumn].addNodeToEnd(cardNode);

                cout << "UNDO SUCCESSFUL: MOVED CARD BACK FROM FOUNDATION TO TABLEAU COLUMN "
                    << move.srcColumn + 1 << "." << endl;
            }
            else {
                cout << "ERROR: FOUNDATION PILE IS EMPTY DURING UNDO." << endl;
            }
            break;
        }

        case Move::MoveWasteToFoundation: {
            // Move the card back from foundation to wastepile
            if (!foundation[move.foundationIndex].isempty()) {
                Node* cardNode = foundation[move.foundationIndex].popNode();
                wastepile.pushNode(cardNode);
                cout << "UNDO SUCCESSFUL: MOVED CARD BACK FROM FOUNDATION TO WASTEPILE." << endl;
            }
            else {
                cout << "ERROR: FOUNDATION PILE IS EMPTY DURING UNDO." << endl;
            }
            break;
        }

        case Move::MoveFoundationToTableau: {
            // Move card back from tableau to foundation
            if (!tableau[move.destColumn].isempty()) {
                Node* cardNode = tableau[move.destColumn].removeLastNode();
                foundation[move.foundationIndex].pushNode(cardNode);
                cout << "UNDO SUCCESSFUL: MOVED CARD BACK FROM TABLEAU TO FOUNDATION PILE "
                    << move.foundationIndex + 1 << "." << endl;
            }
            else {
                cout << "ERROR: TABLEAU COLUMN IS EMPTY DURING UNDO." << endl;
            }
            break;
        }

        case Move::ResetStockFromWaste: {
            // Move all cards from stockpile back to wastepile
            stack tempStack;

            // Reverse the order by pushing stockpile cards into a temporary stack
            while (!stockpile.isempty()) {
                tempStack.pushNode(stockpile.popNode());
            }

            // Move cards back from the temporary stack to wastepile
            while (!tempStack.isempty()) {
                wastepile.pushNode(tempStack.popNode());
            }

            cout << "UNDO SUCCESSFUL: RESET STOCKPILE BACK TO WASTEPILE." << endl;
            break;
        }

        default:
            cout << "UNDO NOT IMPLEMENTED FOR THIS MOVE TYPE." << endl;
            break;
        }
    }

    // Check if the game is won
    bool checkIfGameWon() {
        for (int i = 0; i < 4; ++i) {
            if (foundation[i].isempty() || foundation[i].topItem().rank != 13) {
                return false;
            }
        }
        return true;
    }

    // Check if no more moves are possible
    bool checkIfNoMoreMoves() {
        // Check if there are valid moves between tableau columns
        for (int src = 0; src < 7; ++src) {
            if (tableau[src].isempty()) continue;

            Card srcTopCard = tableau[src].getNodeAt(tableau[src].getsize() - 1)->val;

            // Check if the source card can be moved to any other tableau column
            for (int dest = 0; dest < 7; ++dest) {
                if (dest == src) continue;

                if (tableau[dest].isempty()) {
                    // Check if we can move a King to an empty tableau column
                    if (canMoveToEmptyTableau(srcTopCard)) {
                        return false;
                    }
                }
                else {
                    Card destTopCard = tableau[dest].getNodeAt(tableau[dest].getsize() - 1)->val;

                    // Check if the move is valid based on rank and color
                    if (isonesmaller(srcTopCard, destTopCard) && isOppositeColor(srcTopCard.suit, destTopCard.suit)) {
                        return false;
                    }
                }
            }
        }

        // Check if the wastepile top card can be moved to any tableau column
        if (!wastepile.isempty()) {
            Card wasteTopCard = wastepile.topItem();

            for (int dest = 0; dest < 7; ++dest) {
                if (tableau[dest].isempty()) {
                    if (canMoveToEmptyTableau(wasteTopCard)) {
                        return false;
                    }
                }
                else {
                    Card destTopCard = tableau[dest].getNodeAt(tableau[dest].getsize() - 1)->val;

                    if (isonesmaller(wasteTopCard, destTopCard) && isOppositeColor(wasteTopCard.suit, destTopCard.suit)) {
                        return false;
                    }
                }
            }
        }

        // Check if tableau or wastepile cards can be moved to the foundation
        for (int col = 0; col < 7; ++col) {
            if (!tableau[col].isempty()) {
                Card tableauTopCard = tableau[col].getNodeAt(tableau[col].getsize() - 1)->val;

                for (int f = 0; f < 4; ++f) {
                    if (foundation[f].isempty()) {
                        if (tableauTopCard.rank == 1) return false;
                    }
                    else {
                        Card foundationTopCard = foundation[f].topItem();
                        if (tableauTopCard.suit == foundationTopCard.suit && tableauTopCard.rank == foundationTopCard.rank + 1) {
                            return false;
                        }
                    }
                }
            }
        }

        // Check if the wastepile top card can be moved to the foundation
        if (!wastepile.isempty()) {
            Card wasteTopCard = wastepile.topItem();

            for (int f = 0; f < 4; ++f) {
                if (foundation[f].isempty()) {
                    if (wasteTopCard.rank == 1) return false;
                }
                else {
                    Card foundationTopCard = foundation[f].topItem();
                    if (wasteTopCard.suit == foundationTopCard.suit && wasteTopCard.rank == foundationTopCard.rank + 1) {
                        return false;
                    }
                }
            }
        }

        // Check if stockpile can be drawn
        if (!stockpile.isempty() || !wastepile.isempty()) {
            return false;
        }

        return true;
    }

    // Print the current game state
    void printGameState() const {
        // Display the top of the game board: Stockpile, Wastepile, Foundations
        cout << left;
        cout << setw(10) << "Stock" << setw(10) << "Waste"
            << setw(15) << "Foundation 1" << setw(15) << "Foundation 2"
            << setw(15) << "Foundation 3" << setw(15) << "Foundation 4" << endl;

        // Display stockpile and wastepile
        cout << setw(10) << "[ ]";  // Stockpile display
        if (!wastepile.isempty()) {
            Card wasteTopCard = wastepile.topItem();
            cout << setw(10) << (to_string(wasteTopCard.rank) + wasteTopCard.suit);
        }
        else {
            cout << setw(10) << "[ ]";
        }

        // Display foundations
        for (int i = 0; i < 4; ++i) {
            if (!foundation[i].isempty()) {
                Card foundationTopCard = foundation[i].topItem();
                cout << setw(15) << (to_string(foundationTopCard.rank) + foundationTopCard.suit);  // "1C" format
            }
            else {
                cout << setw(15) << "[ ]";
            }
        }
        cout << endl;

        // Show stockpile and wastepile card counts
        cout << setw(10) << "(" + to_string(stockpile.getsize()) + " cards)";
        cout << setw(10) << "(" + to_string(wastepile.getsize()) + " cards)";

        // Show foundation card counts
        for (int i = 0; i < 4; ++i) {
            cout << setw(15) << "(" + to_string(foundation[i].getsize()) + " cards)";
        }
        cout << "\n\n";

        // Now, print the tableau, showing cards in a vertical alignment
        cout << "Tableau:\n";

        // Print the column numbers
        cout << setw(7) << "Col 1" << setw(7) << "Col 2" << setw(7) << "Col 3"
            << setw(7) << "Col 4" << setw(7) << "Col 5" << setw(7) << "Col 6" << setw(7) << "Col 7" << endl;

        // Calculate and display the number of face-up cards in each tableau column
        for (int i = 0; i < 7; ++i) {
            int faceUpCount = 0;
            Node* current = tableau[i].getHead();
            while (current != nullptr) {
                if (current->val.isFaceUp) {
                    faceUpCount++;
                }
                current = current->next;
            }
            cout << setw(7) << "(" + to_string(faceUpCount) + " up)";
        }
        cout << endl;

        // Get the maximum number of cards in any tableau column
        int maxRows = 0;
        for (int i = 0; i < 7; ++i) {
            int columnSize = tableau[i].getsize();
            if (columnSize > maxRows) {
                maxRows = columnSize;
            }
        }

        // Print the cards in rows, where each row contains one card from each column
        for (int row = 0; row < maxRows; ++row) {
            for (int col = 0; col < 7; ++col) {

                Node* current = tableau[col].getNodeAt(row);
                if (current != nullptr) {
                    if (current->val.isFaceUp) {
                        cout << setw(7) << to_string(current->val.rank) + current->val.suit;
                    }
                    else {
                        cout << setw(7) << "[X]";
                    }
                }
                else {
                    cout << setw(7) << " ";
                }
            }
            cout << endl;
        }

        cout << "\n===============================\n" << endl;
    }

};

//ascii art
void displayMainScreen() {
 
    cout << R"(


                                                                                                                                                                              
                                                                                                                                                                              
   SSSSSSSSSSSSSSS      OOOOOOOOO     LLLLLLLLLLL             IIIIIIIIIITTTTTTTTTTTTTTTTTTTTTTT         AAA               IIIIIIIIIIRRRRRRRRRRRRRRRRR   EEEEEEEEEEEEEEEEEEEEEE
 SS:::::::::::::::S   OO:::::::::OO   L:::::::::L             I::::::::IT:::::::::::::::::::::T        A:::A              I::::::::IR::::::::::::::::R  E::::::::::::::::::::E
S:::::SSSSSS::::::S OO:::::::::::::OO L:::::::::L             I::::::::IT:::::::::::::::::::::T       A:::::A             I::::::::IR::::::RRRRRR:::::R E::::::::::::::::::::E
S:::::S     SSSSSSSO:::::::OOO:::::::OLL:::::::LL             II::::::IIT:::::TT:::::::TT:::::T      A:::::::A            II::::::IIRR:::::R     R:::::REE::::::EEEEEEEEE::::E
S:::::S            O::::::O   O::::::O  L:::::L                 I::::I  TTTTTT  T:::::T  TTTTTT     A:::::::::A             I::::I    R::::R     R:::::R  E:::::E       EEEEEE
S:::::S            O:::::O     O:::::O  L:::::L                 I::::I          T:::::T            A:::::A:::::A            I::::I    R::::R     R:::::R  E:::::E             
 S::::SSSS         O:::::O     O:::::O  L:::::L                 I::::I          T:::::T           A:::::A A:::::A           I::::I    R::::RRRRRR:::::R   E::::::EEEEEEEEEE   
  SS::::::SSSSS    O:::::O     O:::::O  L:::::L                 I::::I          T:::::T          A:::::A   A:::::A          I::::I    R:::::::::::::RR    E:::::::::::::::E   
    SSS::::::::SS  O:::::O     O:::::O  L:::::L                 I::::I          T:::::T         A:::::A     A:::::A         I::::I    R::::RRRRRR:::::R   E:::::::::::::::E   
       SSSSSS::::S O:::::O     O:::::O  L:::::L                 I::::I          T:::::T        A:::::AAAAAAAAA:::::A        I::::I    R::::R     R:::::R  E::::::EEEEEEEEEE   
            S:::::SO:::::O     O:::::O  L:::::L                 I::::I          T:::::T       A:::::::::::::::::::::A       I::::I    R::::R     R:::::R  E:::::E             
            S:::::SO::::::O   O::::::O  L:::::L         LLLLLL  I::::I          T:::::T      A:::::AAAAAAAAAAAAA:::::A      I::::I    R::::R     R:::::R  E:::::E       EEEEEE
SSSSSSS     S:::::SO:::::::OOO:::::::OLL:::::::LLLLLLLLL:::::LII::::::II      TT:::::::TT   A:::::A             A:::::A   II::::::IIRR:::::R     R:::::REE::::::EEEEEEEE:::::E
S::::::SSSSSS:::::S OO:::::::::::::OO L::::::::::::::::::::::LI::::::::I      T:::::::::T  A:::::A               A:::::A  I::::::::IR::::::R     R:::::RE::::::::::::::::::::E
S:::::::::::::::SS    OO:::::::::OO   L::::::::::::::::::::::LI::::::::I      T:::::::::T A:::::A                 A:::::A I::::::::IR::::::R     R:::::RE::::::::::::::::::::E
 SSSSSSSSSSSSSSS        OOOOOOOOO     LLLLLLLLLLLLLLLLLLLLLLLLIIIIIIIIII      TTTTTTTTTTTAAAAAAA                   AAAAAAAIIIIIIIIIIRRRRRRRR     RRRRRRREEEEEEEEEEEEEEEEEEEEEE
                                                                                                                                                                              
                                                                                                                                                                              
                                                                                                                                                                              
                                                                                                                                                                              
                                                                                                                                                                              
                                                                                                                                                                              
                                                                                                                                                                              


    )" << endl;
}

string toLowerCase(const string& str) {
    string lowerStr = str;
    for (size_t i = 0; i < str.size(); ++i) {
        lowerStr[i] = tolower(str[i]);
    }
    return lowerStr;
}

// Command class to handle game start
class Command {
private:
    game solitaireGame;

    void clearScreen() {
        system("CLS");
    }

    // Function to print the game instructions
    void printInstructions() const {
        cout << "Welcome to the Solitaire game!" << endl;
        cout << "Below are the available commands you can use to play the game:" << endl;
        cout << "-------------------------------------------------------------" << endl;
        cout << "s            : Draw a card from the stockpile to the wastepile." << endl;
        cout << "m <src> <dest> <num> : Move 'num' cards from tableau column <src> to tableau column <dest>." << endl;
        cout << "                       - <src> and <dest> are numbers between 1 and 7 (columns in the tableau)." << endl;
        cout << "                       - <num> is the number of cards to move." << endl;
        cout << "                       - Example: 'm 1 3 1' moves the top card from column 1 to column 3." << endl;
        cout << "w2t <col>    : Move the top card from the wastepile to tableau column <col>." << endl;
        cout << "                       - <col> is the destination column number (1-7)." << endl;
        cout << "                       - Example: 'w2t 3' moves the top wastepile card to column 3." << endl;
        cout << "t2f <src> <foundation> : Move the top card from tableau column <src> to foundation <foundation>." << endl;
        cout << "                       - <src> is the tableau column (1-7)." << endl;
        cout << "                       - <foundation> is the foundation pile (1-4)." << endl;
        cout << "                       - Example: 't2f 1 2' moves the top card from column 1 to foundation 2." << endl;
        cout << "w2f <foundation> : Move the top card from the wastepile to foundation <foundation>." << endl;
        cout << "                       - <foundation> is the foundation pile (1-4)." << endl;
        cout << "                       - Example: 'w2f 1' moves the top wastepile card to foundation 1." << endl;
        cout << "f2t <foundation> <col> : Move the top card from foundation <foundation> to tableau column <col>." << endl;
        cout << "                       - <foundation> is the foundation pile (1-4)." << endl;
        cout << "                       - <col> is the tableau column (1-7)." << endl;
        cout << "                       - Example: 'f2t 2 3' moves the top card from foundation 2 to column 3." << endl;
        cout << "z            : Undo the last move." << endl;
        cout << "exit         : Quit the game." << endl;
        cout << "-------------------------------------------------------------" << endl;
    }

public:
    // Constructor to initialize the game
    Command() : solitaireGame() {}

    // Function to process user commands
    void processCommand(string input) {
        clearScreen();  

        input = toLowerCase(input);

        stringstream ss(input);
        string command;
        ss >> command;

        if (command == "s") {
            solitaireGame.drawCardFromStockpile();
        }
        else if (command == "m") {
            int srcColumn, destColumn, numOfCards;
            ss >> srcColumn >> destColumn >> numOfCards;
            if (srcColumn >= 1 && destColumn >= 1) {
                solitaireGame.moveCard(srcColumn - 1, destColumn - 1, numOfCards);
            }
            else {
                cout << "Invalid Columns. Please Use Columns 1 To 7." << endl;
            }
        }
        else if (command == "w2t") {
            int destColumn;
            ss >> destColumn;
            if (destColumn >= 1) {
                solitaireGame.moveFromWasteToTableau(destColumn - 1);
            }
            else {
                cout << "Invalid Column. Please Use Columns 1 To 7." << endl;
            }
        }
        else if (command == "t2f") {
            int srcColumn, foundationIndex;
            ss >> srcColumn >> foundationIndex;
            if (srcColumn >= 1 && foundationIndex >= 1 && foundationIndex <= 4) {
                solitaireGame.moveFromTableauToFoundation(srcColumn - 1, foundationIndex - 1);
            }
            else {
                cout << "Invalid Column Or Foundation. Please Use Columns 1 To 7 And Foundations 1 To 4." << endl;
            }
        }
        else if (command == "w2f") {
            int foundationIndex;
            ss >> foundationIndex;
            if (foundationIndex >= 1 && foundationIndex <= 4) {
                solitaireGame.moveFromWasteToFoundation(foundationIndex - 1);
            }
            else {
                cout << "Invalid Foundation Index. Please Use Foundations 1 To 4." << endl;
            }
        }
        else if (command == "f2t") {
            int foundationIndex, destColumn;
            ss >> foundationIndex >> destColumn;
            if (foundationIndex >= 1 && foundationIndex <= 4 && destColumn >= 1) {
                solitaireGame.moveFromFoundationToTableau(foundationIndex - 1, destColumn - 1);
            }
            else {
                cout << "Invalid Foundation Or Column. Please Use Foundations 1 To 4 And Columns 1 To 7." << endl;
            }
        }
        else if (command == "z") {
            solitaireGame.undoMove();
        }

        else if (command == "exit") {
            cout << "Exiting Game." << endl;
            return;
        }
        else {
            cout << "UNKNOWN COMMAND: " << command << endl;
        }

        printInstructions();
        solitaireGame.printGameState();

       
        if (solitaireGame.checkIfNoMoreMoves()) {
            cout << "No More Valid Moves. Game Over!" << endl;
        }

        
        if (solitaireGame.checkIfGameWon()) {
            cout << "Congratulations! You've Won The Game!" << endl;
        }
    }
};


int main() {
    Command commandProcessor;
    string input;

    // Show the main screen with ASCII art
    displayMainScreen();

    char mainscreeninput;
    cout << "Press Any Alphanumeric key or symbol To Start" << endl;
    cin >> mainscreeninput;

    system("CLS");

    // Main game loop
    while (true) {
        
        cout << "Enter command (s, m, w2t, t2f, w2f, f2t, z, exit): ";
        getline(cin, input); 

    
        commandProcessor.processCommand(input);

     
        if (input == "exit") {
            cout << "Thank you for playing! Goodbye!" << endl;
            break;
        }
    }

    return 0;
}   