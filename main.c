// ReteSocialeRN7SmallOttimizzato4.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//
/*
	La soluzione proposta utilizza per memeorizzare i dati dell'entità una struttura denominata entità.
	In questa struttura sono memorizzati:
									- il nome dell'entità
									per ogni associazione :
												-il numero degli associati
												- i puntatori ad una lista di entità alla quale l'entità ha offerto amicizia
												- un flag che indica se l'entità è da considerare attiva o se è da considerare
												  logicamente cancellata. Questo per rendere più efficente in termini temporali il programma,
												  una volta effettuaa la malloc l'entità viene cancellata solo logicamente e può essere 
												  riutilizzata in seguito senza effettuare una nuova malloc
												- una versione che tiene traccia del numero di volte che l'entità è stata riattivata.
												Poichè non é richiesto il nome degli associati si é scelto di non tenere anche la lista degli amici
												ma solo il loro numero poichè questo è sufficiente per la soluzione del problema.

	Poichè è necessario accedere molto frequentemente alle informazioni delle entità e poichè il numero delle entità è elevato e non noto a priori
	si è scelto di utilizzare una strutura dinamica in cui memorizzare i puntatori alle entità. Il tipo di struttura è stato scelto per redere efficiente
	la ricerca delle entità. Si è utilizzato un albero RN che garantisce una complessità nella ricerca del tipo O log n.

	Nella struttura che implementa il nodo dell'albero sono presenti oltre alle informazioni necessarie al funzionamento dell'albero anche due informazioni
	specifiche della mia implementazione:
									- il puntatore alla struttura dati memorizzata in associazione al nodo
									- un flag che memorizza la versione dell'entità associata al nodo al momento dell'inserimento nell'albero.
									  Questo dato è necessario in quanto nel caso di eliminazione dell'entità utilizzando l'elenco delle entità 
									  di cui l'entità in cancellazione è amica si possono decrementare i contatori delle entità associate.
									  Tuttavia la cancellazione dell'entità lascerebbe nelle entià che hanno offerto amicizia all'entità in cancellazione 
									  riferimenti ad aun entità non più esistente.
									  La versione presente nel nodo e quella dell'entità permettono in caso di riattivazione dell'entità cancellata
									  la corretta gestione del numero degli amici.

    Anche l'elenco delle entità a cui ciascuna entità ha offerto amicizia è memorizzato utilizzando alberi binari RN, i puntatori alle radici di questi alberi 
	sono memorizzati nella struttura dati dell'entità.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

#define ADDENT 0					// costanti simboliche utilizzate per identificare le azioni da eseguire
#define DELENT 1
#define ADDREL 2
#define DELREL 3
#define REPORT 4
#define END    5

#define MAX_CHAR 50					// numero massimo dei caratteri ammessi per il nome di un'entità
#define MAX_RELAZIONI 5				// numero massimo di relazioni definite sull'insieme di entità



typedef struct Node Node;			// anticipazione delle dichiarazioni necessarie pre poter far riferimento alle strutture successivamente dichiarate


typedef struct entita1 {
	char nome[MAX_CHAR];              // identificativo entità
	bool attivo;					  // se true l'entità appartiene alla rete sociale se false è stata eliminata
	short int versione;				  // usata per identificare la versione dell' entità ripristinata
	short int nElementiR[MAX_RELAZIONI];    // numero elementi partecipanti alla relazione
	short int nElementiA[MAX_RELAZIONI];    // numero di elementi 
	Node *pA[MAX_RELAZIONI];         // puntatore alle liste con gli lementi in cui l'entità compare
} entita;




/*------------------------------------- Funzioni di gestione albero binario rosso nero  ------------------*/
enum { RED, BLACK };

typedef int COLOR;
typedef entita *valueType;
typedef struct Node {
	valueType value;
	COLOR color;
	struct Node *right, *left, *parent;
	short int versione;	// utilizzato per identificare la versione dell'entità a cui é associata, 
	                    // per gestire i riferimenti orfani in seguito alla cancellazione dell'entità a cui puntavano
} nodoAlbero;

typedef nodoAlbero* tree;

/*---------------------------------------   Funzioni applicative  ------------------------------------*/

void indiciRelazioniOrdinateAlfabeticamenteInsertionSort(char vetRelazioni[][MAX_CHAR], int indiciOrdinati[MAX_RELAZIONI], int lunghezza);
void stampaListaelementiRelazione(int pRel, entita *e, char vetRelazioni[][MAX_CHAR], int numNodiMaxRel[MAX_RELAZIONI]);
void aggiornaListaEntitaConMaxElementi(tree t, int posRel, int maxElementi, entita *maxRelazioni, int numNodiMaxRel[MAX_RELAZIONI]);
int posizioneRelazione(char *relazione, char vetRelazioni[][MAX_CHAR], int *nRel);




/* ------------------  Implementazione gestione albero rosso nero -----------------------------*/

nodoAlbero *initialize(valueType v)
{
	nodoAlbero *n = (nodoAlbero *)malloc(sizeof(struct Node));
	n->left = n->right = NULL;
	n->parent = NULL;
	n->value = v;
	n->color = RED;
	return n;
}

bool isOnLeft(nodoAlbero *t);
void fixDoubleBlack(tree *root, nodoAlbero *x);



// check if node is left child of parent 
bool isOnLeft(nodoAlbero *t) { return t == t->parent->left; }

// returns pointer to uncle 
nodoAlbero *uncleN(nodoAlbero *t) {
	// If no parent or grandparent, then no uncle 
	if (t->parent == NULL || t->parent->parent == NULL)
		return NULL;

	if (isOnLeft(t->parent))
		// uncle on right 
		return t->parent->parent->right;
	else
		// uncle on left 
		return t->parent->parent->left;
}



// returns pointer to sibling 
nodoAlbero *siblingN(nodoAlbero *t) {
	// sibling null if no parent 
	if (t->parent == NULL)
		return NULL;

	if (isOnLeft(t))
		return t->parent->right;

	return t->parent->left;
}

// moves node down and moves given node in its place 
void moveDown(nodoAlbero *nParent, nodoAlbero *t) {
	if (t->parent != NULL) {
		if (isOnLeft(t)) {
			t->parent->left = nParent;
		}
		else {
			t->parent->right = nParent;
		}
	}
	nParent->parent = t->parent;
	t->parent = nParent;
}

bool hasRedChild(nodoAlbero *t) {
	return (t->left != NULL && t->left->color == RED) ||
		(t->right != NULL && t->right->color == RED);
}




// left rotates the given node 
void leftRotate(tree *root, nodoAlbero *x) {
	// new parent will be node's right child 
	nodoAlbero *nParent = x->right;

	// update root if current node is root 
	if (x == *root)
		*root = nParent;

	moveDown(nParent, x);

	// connect x with new parent's left element 
	x->right = nParent->left;
	// connect new parent's left element with nodoAlbero 
	// if it is not null 
	if (nParent->left != NULL)
		nParent->left->parent = x;

	// connect new parent with x 
	nParent->left = x;
}

void rightRotate(tree *root, nodoAlbero *x) {
	// new parent will be node's left child 
	nodoAlbero *nParent = x->left;

	// update root if current node is root 
	if (x == *root)
		*root = nParent;

	moveDown(nParent, x);

	// connect x with new parent's right element 
	x->left = nParent->right;
	// connect new parent's right element with nodoAlbero 
	// if it is not null 
	if (nParent->right != NULL)
		nParent->right->parent = x;

	// connect new parent with x 
	nParent->right = x;
}

void swapColors(nodoAlbero *x1, nodoAlbero *x2) {
	COLOR temp;
	temp = x1->color;
	x1->color = x2->color;
	x2->color = temp;
}

void swapValues(nodoAlbero *u, nodoAlbero *v) {
	entita *temp;
	temp = u->value;
	u->value = v->value;                          // modificare in funzione della struttura dati
	v->value = temp;                              
	int tempVers;
	tempVers = u->versione;                       // modificare in funzione della struttura dati
	u->versione = v->versione;
	v->versione = tempVers;
}

// fix red red at given node 
void fixRedRed(tree *root, nodoAlbero *x) {
	// if x is root color it black and return 
	if (x == *root) {
		x->color = BLACK;
		return;
	}

	// initialize parent, grandparent, uncle 
	nodoAlbero *parent = x->parent, *grandparent = parent->parent;
	nodoAlbero *uncle = uncleN(x);

	if (parent->color != BLACK) {
		if (uncle != NULL && uncle->color == RED) {
			// uncle red, perform recoloring and recurse 
			parent->color = BLACK;
			uncle->color = BLACK;
			grandparent->color = RED;
			fixRedRed(root, grandparent);
		}
		else {
			// Else perform LR, LL, RL, RR 
			if (isOnLeft(parent)) {
				if (isOnLeft(x)) {
					// for left right 
					swapColors(parent, grandparent);
				}
				else {
					leftRotate(root, parent);
					swapColors(x, grandparent);
				}
				// for left left and left right 
				rightRotate(root, grandparent);
			}
			else {
				if (isOnLeft(x)) {
					// for right left 
					rightRotate(root, parent);
					swapColors(x, grandparent);
				}
				else {
					swapColors(parent, grandparent);
				}

				// for right right and right left 
				leftRotate(root, grandparent);
			}
		}
	}
}

// find node that do not have a left child 
// in the subtree of the given node 
nodoAlbero *successor(nodoAlbero *x) {
	nodoAlbero *temp = x;

	while (temp->left != NULL)
		temp = temp->left;

	return temp;
}

// find node that replaces a deleted node in BST 
nodoAlbero *BSTreplace(nodoAlbero *x) {
	// when node have 2 children 
	if (x->left != NULL && x->right != NULL)
		return successor(x->right);

	// when leaf 
	if (x->left == NULL && x->right == NULL)
		return NULL;

	// when single child 
	if (x->left != NULL)
		return x->left;
	else
		return x->right;
}

// deletes the given node 
void deleteNode(tree *root, nodoAlbero *v) {
	nodoAlbero *u = BSTreplace(v);

	// True when u and v are both black 
	bool uvBlack = ((u == NULL || u->color == BLACK) && (v->color == BLACK));
	nodoAlbero *parent = v->parent;

	if (u == NULL) {
		// u is NULL therefore v is leaf 
		if (v == *root) {
			// v is root, making root null 
			*root = NULL;
		}
		else {
			if (uvBlack) {
				// u and v both black 
				// v is leaf, fix double black at v 
				fixDoubleBlack(root, v);
			}
			else {
				// u or v is red 
				if (siblingN(v) != NULL)
					// sibling is not null, make it red" 
					siblingN(v)->color = RED;
			}

			// delete v from the tree 
			if (isOnLeft(v)) {
				parent->left = NULL;
			}
			else {
				parent->right = NULL;
			}
		}
		free(v);
		return;
	}

	if (v->left == NULL || v->right == NULL) {
		// v has 1 child 
		if (v == *root) {
			// v is root, assign the value of u to v, and delete u 
			v->value = u->value;               // modificare in funzione della struttura dati 
			v->versione = u->versione;         // modificare in funzione della struttura dati
			v->left = v->right = NULL;
			free(u);
		}
		else {
			// Detach v from tree and move u up 
			if (isOnLeft(v)) {
				parent->left = u;
			}
			else {
				parent->right = u;
			}
			free(v);
			u->parent = parent;
			if (uvBlack) {
				// u and v both black, fix double black at u 
				fixDoubleBlack(root, u);
			}
			else {
				// u or v red, color u black 
				u->color = BLACK;
			}
		}
		return;
	}

	// v has 2 children, swap values with successor and recurse 
	swapValues(u, v);
	deleteNode(root, u);
}

void fixDoubleBlack(tree *root, nodoAlbero *x) {
	if (x == *root)
		// Reached root 
		return;

	nodoAlbero *sibling;
	sibling = siblingN(x);

	nodoAlbero *parent = x->parent;

	if (sibling == NULL) {
		// No sibiling, double black pushed up 
		fixDoubleBlack(root, parent);
	}
	else {
		if (sibling->color == RED) {
			// Sibling red 
			parent->color = RED;
			sibling->color = BLACK;
			if (isOnLeft(sibling)) {
				// left case 
				rightRotate(root, parent);
			}
			else {
				// right case 
				leftRotate(root, parent);
			}
			fixDoubleBlack(root, x);
		}
		else {
			// Sibling black 
			if (hasRedChild(sibling)) {
				// at least 1 red children 
				if (sibling->left != NULL && sibling->left->color == RED) {
					if (isOnLeft(sibling)) {
						// left left 
						sibling->left->color = sibling->color;
						sibling->color = parent->color;
						rightRotate(root, parent);
					}
					else {
						// right left 
						sibling->left->color = parent->color;
						rightRotate(root, sibling);
						leftRotate(root, parent);
					}
				}
				else {
					if (isOnLeft(sibling)) {
						// left right 
						sibling->right->color = parent->color;
						leftRotate(root, sibling);
						rightRotate(root, parent);
					}
					else {
						// right right 
						sibling->right->color = sibling->color;
						sibling->color = parent->color;
						leftRotate(root, parent);
					}
				}
				parent->color = BLACK;
			}
			else {
				// 2 black children 
				sibling->color = RED;
				if (parent->color == BLACK)
					fixDoubleBlack(root, parent);
				else
					parent->color = BLACK;
			}
		}
	}
}



// prints inorder recursively 
void inorder(nodoAlbero *x) {
	if (x == NULL)
		return;
	inorder(x->left);
	printf("%s ", x->value->nome);
	inorder(x->right);
}


nodoAlbero *cercaInListaRelazioni(tree root, entita *e)
{
	nodoAlbero *temp = root;

	while (temp != NULL) {
		if (strcmp(e->nome, temp->value->nome) < 0) {
			if (temp->left == NULL)
			{
				temp = NULL;
				break;
			}
			else
				temp = temp->left;
		}
		else if (strcmp(e->nome, temp->value->nome) == 0) {
			break;
		}
		else {
			if (temp->right == NULL)
			{
				temp = NULL;
				break;
			}
			else
				temp = temp->right;
		}
	}

	return temp;
}


// searches for given value 
// if found returns the node (used for delete) 
// else returns NULL 
// if attivo == false node was found but it is logically deleted
nodoAlbero *findNodeByName(tree root, char *nome, bool *attivo) {
	nodoAlbero *temp = root;
	*attivo = true;
	while (temp != NULL) {
		if (strcmp(nome, temp->value->nome) < 0) {
			if (temp->left == NULL)
			{
				temp = NULL;
				break;
			}
			else
				temp = temp->left;
		}
		else if (strcmp(nome, temp->value->nome) == 0) {
			if (temp->value->attivo == false)  // se il nodo è presente ma è marcato come eliminato ritorna 
				*attivo = false;
			break;
		}
		else {
			if (temp->right == NULL)
			{
				temp = NULL;
				break;
			}
			else
				temp = temp->right;
		}
	}

	return temp;
}

// searches for given value 
// if found returns the node (used for delete) 
// else returns the last node while traversing (used in insert) 
nodoAlbero *findNode(tree root, char *nome) {
	nodoAlbero *temp = root;
	while (temp != NULL) {
		if (strcmp(nome, temp->value->nome) < 0) {
			if (temp->left == NULL)
				break;
			else
				temp = temp->left;
		}
		else if (strcmp(nome, temp->value->nome) == 0) {
			break;
		}
		else {
			if (temp->right == NULL)
				break;
			else
				temp = temp->right;
		}
	}

	return temp;
}

void inserisciInListaRelazioni(tree *root, entita *e)
{
	nodoAlbero *newNode = (nodoAlbero *)malloc(sizeof(struct Node));
	newNode->value = e;
	newNode->left = NULL;
	newNode->parent = NULL;
	newNode->right = NULL;
	newNode->color = RED;
	newNode->versione = e->versione;

	if (*root == NULL) {
		// when root is null 
		// simply insert value at root 
		newNode->color = BLACK;
		*root = newNode;
	}
	else {
		nodoAlbero *temp = findNode(*root, e->nome);

		if (strcmp(e->nome, temp->value->nome) == 0) {
			// return if value already exists 
			return;
		}

		// if value is not found, search returns the node 
		// where the value is to be inserted 

		// connect new node to correct node 
		newNode->parent = temp;

		if (strcmp(e->nome, temp->value->nome) < 0)
			temp->left = newNode;
		else
			temp->right = newNode;

		// fix red red voilaton if exists 
		fixRedRed(root, newNode);
	}
}

// inserts the given value to tree 
void insertNode(tree *root, entita*e) {

	nodoAlbero *newNode = (nodoAlbero *)malloc(sizeof(struct Node));
	newNode->value = e;
	newNode->versione = e->versione;
	newNode->left = NULL;
	newNode->parent = NULL;
	newNode->right = NULL;
	newNode->color = RED;

	if (*root == NULL) {
		// when root is null 
		// simply insert value at root 
		newNode->color = BLACK;
		*root = newNode;
	}
	else {
		nodoAlbero *temp = findNode(*root, e->nome);

		if (strcmp(e->nome, temp->value->nome) == 0) {
			// return if value already exists 
			return;
		}

		// if value is not found, search returns the node 
		// where the value is to be inserted 

		// connect new node to correct node 
		newNode->parent = temp;

		if (strcmp(e->nome, temp->value->nome) < 0)
			temp->left = newNode;
		else
			temp->right = newNode;

		// fix red red voilaton if exists 
		fixRedRed(root, newNode);
	}
}

// utility function that deletes the node with given value 
void deleteByVal(tree *root, char *n) {
	if (root == NULL)
		// Tree is empty 
		return;

	nodoAlbero *v = findNode(*root, n);

	if (strcmp(v->value->nome, n) != 0) {
		printf("No node found to delete with value:\n");
		return;
	}

	deleteNode(root, v);
}

// prints inorder of the tree 
void inorderPrint(tree root) {
	printf("Inorder: \n");
	if (root == NULL)
		printf("Tree is empty\n");
	else
		inorder(root);
	printf("\n");;
}

void printNode(nodoAlbero *n)
{
	printf("%s(%s) n:%d", n->value->nome, (n->color == BLACK ? "b" : "r"), n->value->nElementiR[0]);
}

void destroy(tree tree)
{
	if (tree == NULL)
		return;
	destroy(tree->left);
	destroy(tree->right);
	free(tree);
}

void rimuoviTutteLeRelazioni(tree alberoRelazioni)
{
	if(alberoRelazioni!= NULL)
		destroy(alberoRelazioni);
}


void removeNodeByName(tree *t, char *nome)
{
	bool attivo;
	nodoAlbero *v = findNodeByName(*t, nome, &attivo);

	if (v != NULL && attivo == true)
	{
		v->value->attivo = false;  // elimino logicamente il nodo
		if (v->value != NULL)
		{

			// rilascio i dati delle liste assciate all'entità
			for (int i = 0; i < MAX_RELAZIONI; i++)
			{
				v->value->nElementiR[i] = 0;
				v->value->nElementiA[i] = 0;
				if (v->value->pA[i] != NULL)
				{
					rimuoviTutteLeRelazioni((tree)v->value->pA[i]);
					v->value->pA[i] = NULL;
				}
			}
			// rilasio i dati dell'entita'
			//free(v->value);
		}
		//deleteNode(t, v);
	}
}

void rimuoviNodoListaRelazioni(tree *t, nodoAlbero *v)
{
	if (v != NULL)
	{
		if (v->value->attivo == true && v->versione == v->value->versione)   // il nodo deve essere cancellato solo se la sua versione coincide cona la versione attuale dell'entità a cui è associato'
		{
			deleteNode(t, v);
		}

	}
}



void removeNodeByPointer(tree *t, nodoAlbero *v)
{

	if (v != NULL)
	{

		if (v->value != NULL)
		{
			v->value->attivo = false;
			// rilascio i dati delle liste assciate all'entità
			for (int i = 0; i < MAX_RELAZIONI; i++)
			{
				v->value->nElementiR[i] = 0;
				v->value->nElementiA[i] = 0;
				if (v->value->pA[i] != NULL)
				{
					rimuoviTutteLeRelazioni((tree) v->value->pA[i]);
					v->value->pA[i] = NULL;
				}
			}
			// rilasio i dati dell'entita'
			//free(v->value);
		}
		//deleteNode(t, v); Il nodo dell'albero e l'entità associata vengono mantenute e possono essere riutilizzate
	}
}



void liberaMemoria(tree tree)
{
	if (tree == NULL)
		return;
	liberaMemoria(tree->left);
	liberaMemoria(tree->right);
	// svuoto le liste delle relazioni dell'entità

	for (int i = 0; i < MAX_RELAZIONI; i++)
	{
		if (tree->value->pA[i] != NULL)
		{
			rimuoviTutteLeRelazioni(tree->value->pA[i]);
			tree->value->pA[i] = NULL;
		}
	}
	// rilascio la memoria dell'entità
	free(tree->value);
	// rilascio il nodo dell'albero
	free(tree);
}


void deallocaEntita(entita *e)
{
	if (e == NULL)
		return;
	
	// svuoto le liste delle relazioni dell'entità

	for (int i = 0; i < MAX_RELAZIONI; i++)
	{
		if (e->pA[i] != NULL)
		{
			rimuoviTutteLeRelazioni((tree) e->pA[i]);
			e->pA[i] = NULL;
		}
	}
	// rilascio la memoria dell'entità
	free(e);

}


// la funzione ritorna il massimo numero di entità coinvolte in una relazione identificata dalla 
// sua posizione nella tabella delle relazioni
void BSTInorderMAX(tree t, int posRel, int *maxElementi)
{

	if (t != NULL) {
		BSTInorderMAX(t->left, posRel, maxElementi);
		if (t->value->attivo == true && t->value->nElementiR[posRel] > *maxElementi)
			*maxElementi = t->value->nElementiR[posRel];
		BSTInorderMAX(t->right, posRel, maxElementi);
	}
}





/* la funzione identifica la relazione e ne ritorna la posizione all'interno
del vettore delle relazioni, se la relazione non esite viene inserita nel vettore*/
int posizioneRelazione(char *relazione, char vetRelazioni[][MAX_CHAR], int *nRel)
{
	int i;
	bool trovato = false;
	if (*nRel < MAX_RELAZIONI)
	{
		for (i = 0; i < *nRel && trovato == false; i++)
		{
			if (strcmp(relazione, vetRelazioni[i]) == 0)
			{
				trovato = true;
				return i;
			}
		}
		// la relazione non è presente, la aggiungo al vettore
		i = 0;
		while (relazione[i] != '\0')
		{
			vetRelazioni[*nRel][i] = relazione[i];
			i++;
		}
		vetRelazioni[*nRel][i] = '\0';

		//strcpy(&vetRelazioni[*nRel][0], relazione);
		(*nRel)++;
		return(*nRel - 1);
	}
	else
		return -1; // non vi è spazio nell vettore per memorizzare la nuova relazione

}



/*
	La funzione aggiorna le liste dell'entità fittizia utilizzata per contenere per ciascuna relazione
	l'elenco delle entità che in un dato istante hanno il maggior numero di entità associate

	La relazione è identificata dal numero della relazione posRel

	La funzione riceve in ingresso il putatore alla radice dell'albero ed il numero massimo di elementi
	della relazione selezionata.
	La lista viene costruita con una scansione completa dell'albero selezionando le entità che hanno
	associate il numero di lementi massimo passato in ingresso alla funzione
*/
void aggiornaListaEntitaConMaxElementi(tree t, int pRel, int maxElementi, entita *maxRelazioni, int numNodiMaxRel[MAX_RELAZIONI])
{

	if (t != NULL) {
		aggiornaListaEntitaConMaxElementi(t->left, pRel, maxElementi, maxRelazioni, numNodiMaxRel);
		if (t != NULL)
		{
			if (t->value->nElementiR[pRel] == maxElementi)
			{
				if (t->value->attivo == true)
				{
					// verifico che l'entità non compaia gia' nella relazione
					if (cercaInListaRelazioni((tree)maxRelazioni->pA[pRel], t->value) == NULL)
					{
						//esistono già elementi per il tipo di relazione aggiungo un nodo
//						if (maxRelazioni->pA[pRel] != NULL)
						{
							inserisciInListaRelazioni(&maxRelazioni->pA[pRel], t->value);
							//maxRelazioni->nElementiA[pRel]++;     // incremento il numero di elementi della relazione
							numNodiMaxRel[pRel]++;
						}
					}
					maxRelazioni->nElementiA[pRel] = maxElementi;
					//maxRelazioni->nElementiR[pRel] = maxElementi;
					//numNodiMaxRel[pRel]++;
				}
			}
		}
		aggiornaListaEntitaConMaxElementi(t->right, pRel, maxElementi, maxRelazioni, numNodiMaxRel);
	}
}


// SOSTITUIRE QUESTA ALLE DUE FUNZIONI BSTINorder e a aggiornaListaEntitaConMaxElementi
// USARE un falg che invalida lista solo se è stata rimossa una relazione o una entità e che la valida all'inizio o quando viene eseguita
// questa procedura
void aggiornaListaUtentiConMaxRelazioni(tree t, entita* lista, int massimo[MAX_RELAZIONI], int nRel)
{
	int i, elementiRelazione;


	if (t == NULL)
		return;
	else {
		aggiornaListaUtentiConMaxRelazioni(t->left, lista, massimo, nRel);
		for (i = 0; i < nRel; i++) {
			elementiRelazione = t->value->nElementiR[i];
			if (elementiRelazione >= massimo[i])
			{
				// aggiungo alla lista delle entità con il max numero di elementi
				if (elementiRelazione == massimo[i])
				{
					//					if (lista->pR[i] == NULL) {
					//						LSTinizializza(&lista->pR[i]);
					//					}
					inserisciInListaRelazioni(&lista->pA[i], t->value);
				}
				else  // nuovo massimo - svuoto lista e riparto da capo
				{
					massimo[i] = elementiRelazione;
					if (lista->pA[i] != NULL) {
						rimuoviTutteLeRelazioni((tree)lista->pA[i]);
						lista->pA[i] = NULL;
					}

					if (lista->pA[i] != NULL)
						inserisciInListaRelazioni(&lista->pA[i], t->value);
				}
			}
		}
		aggiornaListaUtentiConMaxRelazioni(t->right, lista, massimo, nRel);
	}
}


// prints inorder of the tree 
void stampaOrdinatElementiRelazione(tree t) {

	if (t != NULL) {
		stampaOrdinatElementiRelazione(t->left);
			fputs(t->value->nome, stdout);
			fputs(" ", stdout);
		stampaOrdinatElementiRelazione(t->right);
	}
}


// prints inorder of the tree 
void aggiornaAmiciInCancellazioneEntita(tree t, int pRel) {

	if (t != NULL) {
		aggiornaAmiciInCancellazioneEntita(t->left,pRel);
		if (t->value->attivo == true && t->value->versione == t->versione)
			t->value->nElementiR[pRel]--;                    // tolgo un'amico all'entità di cui sono amica
		aggiornaAmiciInCancellazioneEntita(t->right,pRel);
	}
}
/*
	La funzione produce l'output del programma su standard output

	Riceve in ingresso :
	- e l'entità fittizia che contiene la lista delle entità con il numero massimo
	- pRel l'identificativo numerico della relazione
	- il vettore con il nome delle relazioni vetRelazioni
	- il vettore con il numero massimo dei nodi per ciascuna relazione numMaxNodiRelazioni

*/
void stampaListaelementiRelazione(int pRel, entita *e, char nomiRelazioni[][MAX_CHAR], int numMaxNodiRelazioni[MAX_RELAZIONI])
{
	Node *L;

	L = e->pA[pRel];
	if (L != NULL)
	{
		//printf("\"%s\" ", &nomiRelazioni[pRel][0]);
		fputs(&nomiRelazioni[pRel][0], stdout);
		fputs(" ", stdout);
		{
			stampaOrdinatElementiRelazione(e->pA[pRel]);
		}
		printf("%d; ", e->nElementiR[pRel]);

	}
}




void indiciRelazioniOrdinateAlfabeticamenteInsertionSort(char vetRelazioni[][MAX_CHAR], int indiciOrdinati[MAX_RELAZIONI], int lunghezza)
{
	int i, j, t, k;
	char temp[MAX_CHAR];

	char vetLocaleNomiRelazioni[MAX_RELAZIONI][MAX_CHAR];

	// copio il vettore dei nomi originari nel vettore d'appoggio per effettuare l'ordinamento
	for (i = 0; i < lunghezza; i++)
	{
		//strcpy(&vetLocaleNomiRelazioni[i][0], &vetRelazioni[i][0]);
		k = 0;
		while (vetRelazioni[i][k] != '\0')
		{
			vetLocaleNomiRelazioni[i][k] = vetRelazioni[i][k];
			k++;
		}
		vetLocaleNomiRelazioni[i][k] = '\0';
	}

	for (i = 0; i < lunghezza; i++)                     //Loop for ascending ordering
	{
		for (j = 0; j < lunghezza; j++)					//Loop for comparing other values
		{
			if (strcmp(&vetLocaleNomiRelazioni[j][0], &vetLocaleNomiRelazioni[i][0]) > 0)							//Comparing other array elements
			{
				t = indiciOrdinati[i];
				k = 0;
				while (vetLocaleNomiRelazioni[i][k] != '\0')
				{
					temp[k] = vetLocaleNomiRelazioni[i][k];
					k++;
				}
				vetLocaleNomiRelazioni[i][k] = '\0';
				temp[k] = '\0';
				//strcpy(temp, &vetLocaleNomiRelazioni[i][0]);
				k = 0;
				while (vetLocaleNomiRelazioni[j][k] != '\0')
				{
					vetLocaleNomiRelazioni[i][k] = vetLocaleNomiRelazioni[j][k];
					k++;
				}
				vetLocaleNomiRelazioni[i][k] = '\0';
				//strcpy(&vetLocaleNomiRelazioni[i][0], &vetLocaleNomiRelazioni[j][0]);
				k = 0;
				while (temp[k] != '\0')
				{
					vetLocaleNomiRelazioni[j][k] = temp[k];
					k++;
				}
				vetLocaleNomiRelazioni[j][k] = '\0';
				//strcpy(&vetLocaleNomiRelazioni[j][0], temp);
				indiciOrdinati[i] = indiciOrdinati[j];
				indiciOrdinati[j] = t;
			}
		}
	}
}


/*

		Utilizzando l'informazione che la fomattazione delle righe di comando si é deciso di non usare le funzioni strtok e strcpy
		per otenere la massima efficienza.

		La funzione restituisce i nomi delle entità coinvolte nel comando, i puntatori ai nodi dell'albero che le indirizza
		ed un flag che se false indica che l'entità è nello stato di cancellato

*/
int decodificaInput(tree reteSociale, char *buf, char *p1, char *p2, char *p3, nodoAlbero  **punt1, nodoAlbero  **punt2, bool *attivo)
{
	int azione;

	if (buf[0] == 'a' && buf[5] == 't')
		azione = ADDENT;
	else if (buf[0] == 'd' && buf[5] == 't')
		azione = DELENT;
	else if (buf[0] == 'a' && buf[5] == 'l')
		azione = ADDREL;
	else if (buf[0] == 'd' && buf[5] == 'l')
		azione = DELREL;
	else if (buf[0] == 'r')
		azione = REPORT;
	else azione = END;

	int j = 0, i = 7;


	switch (azione)
	{
	case ADDENT:
		// estraggo il nome dell'entità se non è presente termino
		while (buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n')
		{
			p1[j] = buf[i];
			j++;
			i++;
		}
		p1[j] = '\0';
		*punt1 = findNodeByName(reteSociale, p1, attivo);

		// cerco il puntatore dell'entità nell'albero se non viene trovato creo ed inserisco l'entita
		break;
	case DELENT:
		while (buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n')
		{
			p1[j] = buf[i];
			j++;
			i++;
		}
		p1[j] = '\0';
		*punt1 = findNodeByName(reteSociale, p1, attivo);
		break;
	case ADDREL:
		while (buf[i] != ' ')
		{
			p1[j] = buf[i];
			j++;
			i++;
		}
		p1[j] = '\0';
		*punt1 = findNodeByName(reteSociale, p1, attivo);
		if (punt1 == NULL)
			break;
		j = 0;
		i++;
		while (buf[i] != ' ')
		{
			p2[j] = buf[i];
			j++;
			i++;
		}
		p2[j] = '\0';
		*punt2 = findNodeByName(reteSociale, p2, attivo);
		if (punt2 == NULL)
			break;
		j = 0;
		i++;
		while (buf[i] != ' '  && buf[i] != '\0' && buf[i] != '\n')
		{
			p3[j] = buf[i];
			j++;
			i++;
		}
		p3[j] = '\0';
		break;
	case DELREL:
		while (buf[i] != ' ')
		{
			p1[j] = buf[i];
			j++;
			i++;
		}
		p1[j] = '\0';
		*punt1 = findNodeByName(reteSociale, p1, attivo);
		if (punt1 == NULL)
			break;
		j = 0;
		i++;
		while (buf[i] != ' ')
		{
			p2[j] = buf[i];
			j++;
			i++;
		}
		p2[j] = '\0';
		*punt2 = findNodeByName(reteSociale, p2, attivo);
		if (punt2 == NULL)
			break;
		j = 0;
		i++;
		while (buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\n')
		{
			p3[j] = buf[i];
			j++;
			i++;
		}
		p3[j] = '\0';
		break;
	}

	return(azione);
}

int main()
{


	int azione;
	char buf[200];
	char p1[100], p2[100], p3[100];
	char *res;
	entita *e;
	entita *maxRelazioni;                        // puntatore ad un entità fittizia che per ogni relazione 
												 // contiene il numero massimo di entità collegate ed i puntatori
												 // alle entità che hanno un numero di elmenti associati pari al 
												 // massimo, tali valori sono aggiornati ogni volta che si stabilisce
												 // o si cancella una relazione o entita'
	Node *L;									 // utilizzato per l'output finale
	char vetRelazioni[MAX_RELAZIONI][MAX_CHAR];  // matrice contenete il nome delle relazioni 
	int numNodiMaxRel[MAX_RELAZIONI];            // contiene il numero di nodi che hanno il numero massimo di elementi 
												 // costruita analizzando i nomi trovati nei comandi
	int indiciOrdinati[MAX_CHAR];				 // contiene gli indici della posizione nella matrice dei nomi delle relazioni delle relazioni ordinate alfabeticamente
	int indice;									 // utilizzato per la stampa ordinata dei nomi delle relazioni

	int numRelazioni = 0;                        // contiene il numero di relazioni presenti nella matrice
	int oldNumeroRelazioni = 0;					 // per ordinare il vettore dei nomi delle relazioni solo se viene introdotta una nuova relazione
	bool nomiOrdinati = false;					 // per decidere se effetture l'ordinamento dei nomi delle relazioni

	int stampati;
	int pRel;                                    // posizione della relazione nel vettore delle relazioni
	int i;
	int numEntitaRel;
	int maxElementiRel;
	bool attivo;								 // se true il nodo dell'albero è logicament attivo altrimenti e' da considerarsi cancellato	
	bool esci = false;							 // se true termina il programma

	tree  reteSociale = NULL;                    // albero contenete le entità della rete sociale
	nodoAlbero  *punt1, *punt2;        // puntatori ai nodi che offrono e ricevono amicizia
	bool inserisci;
	bool riusaNodo;

	maxRelazioni = (entita *)malloc(sizeof(entita));  // alloco l'entità fittiza che contiene l'elenco delle entità che per ogni associaizone hanno il massimo numero di associati
	maxRelazioni->nome[0] = 'm';
	maxRelazioni->nome[1] = '\0';
	maxRelazioni->attivo = true;
	maxRelazioni->versione = 0;

	for (int i = 0; i < MAX_RELAZIONI; i++)			// inizializzazione dei valori 
	{
		maxRelazioni->nElementiA[i] = 0;
		maxRelazioni->nElementiR[i] = 0;
		maxRelazioni->pA[i] = NULL;
		numNodiMaxRel[i] = 0;
	}

	riusaNodo = false;
	punt1 = NULL;
	punt2 = NULL;



	while (esci==false) {

		res = gets(buf);       // leggo da file il comando 
		if (res == NULL)
			break;

		azione=decodificaInput(reteSociale, buf, p1, p2, p3, &punt1, &punt2, &attivo);    // decodifico ed eseguo il comando


		switch (azione)                    // punt2 contiene il puntatore all'entità che riceve l'amicizia, punt1 il puntatore all'entità che la offre

		{
		case ADDENT:  // alloco la nuova entità e la inserisco nell'albero

			if (punt1 == NULL)
			{
				e = (entita*)malloc(sizeof(entita));
				i = 0;
				while (p1[i] != '\0')
				{
					e->nome[i] = p1[i];
					i++;
				}
				e->nome[i] = '\0';
				e->attivo = true;
				e->versione = 0;
				for (i = 0; i < MAX_RELAZIONI; i++)
				{
					e->nElementiA[i] = 0;
					e->nElementiR[i] = 0;
					e->pA[i] = NULL;
				}
				insertNode(&reteSociale, e);
			}
			else
			{
				// il nodo è gia' presente nell'albero ma è logicamente cancellato
				// lo riattivo
				if (punt1->value->attivo == false)
				{
					punt1->value->attivo = true;
					punt1->value->versione++;       // se riattivo il nodo cambio il suo numero di versione
				}

			}
			break;
		case DELENT:  // Cancello logicamnete l'entità

			if (punt1 != NULL && punt1->value->attivo == true)
			{

				entita *p1;

				int maxElementiRel;
				bool ricalcolaListaElementiconMaxAmici;

				ricalcolaListaElementiconMaxAmici = false;
				// entità da eliminare
				p1 = punt1->value;

				// da tutte le entità con le quali l'entità in eliminazione ha amicizia decremento di uno il numero degli amici
				// rilascio la lista degli amici e cancello logicamente l'entità
				for (i = 0; i < numRelazioni; i++)
				{
					if (p1->nElementiA[i] > 0)
					{
						ricalcolaListaElementiconMaxAmici = true;  // ricalcolo solo se l'entità aveva qualche associazione

						Node *L;

						L = p1->pA[i];
						aggiornaAmiciInCancellazioneEntita(L, i);
					}
					else
						if (p1->nElementiA[i] == 0 && p1->nElementiR[i] > 0)
							ricalcolaListaElementiconMaxAmici = true;
				}
				// elimino l'entità dalla rete sociale
				removeNodeByPointer(&reteSociale, punt1);

				// rivisito l'albero per aggiornare la variabile che contiene la lista delle variabili con il massimo numero di elementi
				// non effettuo l'operazione se l'entità cancellata non aveva associazioni
				if (ricalcolaListaElementiconMaxAmici == true)
				{
					for (i = 0; i < numRelazioni; i++)
					{
						maxElementiRel = 0;
						BSTInorderMAX(reteSociale, i, &maxElementiRel);
						maxRelazioni->nElementiR[i] = 0;
						maxRelazioni->nElementiA[i] = 0;
						numNodiMaxRel[i] = 0;
						rimuoviTutteLeRelazioni((tree)maxRelazioni->pA[i]);
						maxRelazioni->pA[i] = NULL;
						if (maxElementiRel > 0)
							aggiornaListaEntitaConMaxElementi(reteSociale, i, maxElementiRel, maxRelazioni, numNodiMaxRel); // FORSE è meglio non farlo e farlo solo quando viene richiesto il report

					}
				}
			}
			break;
		case ADDREL:  // inserimento di un associazione

			if (punt1 != NULL && punt2 != NULL)
			{
				if (punt1->value->attivo == true && punt2->value->attivo == true)
				{
					riusaNodo = false;
					Node *puntRel = NULL;
					inserisci = false;
					// ottengo la posizione della relazione nel vettore delle relazioni
					// se non è già presente viene aggiunta
					pRel = posizioneRelazione(p3, vetRelazioni, &numRelazioni);
					// verifico che la relazione non esista già
					if (punt1->value->pA[pRel] == NULL)
					{
						inserisci = true;
					}
					else
					{
						puntRel = cercaInListaRelazioni((tree) punt1->value->pA[pRel], punt2->value);
						if (puntRel == NULL)
							inserisci = true;
						else
							if (puntRel->versione != punt2->value->versione)  // il nodo esiste già ma appartine ad una entità deallocata, lo riuso
								riusaNodo = true;
					}

					if (inserisci == true || riusaNodo == true)
					{
						// incremento il numero dei soggetti in relazione con l'entita

						punt2->value->nElementiR[pRel]++;

						// creo un nodo e lo aggiungo all'albero delle entità con cui il nodo ha 
						// stabilito una relazione, questa informazione è utilizzata quando
						// si cancella un'entità per decrementare il nmeor degli amic nelle entità associate
						if (punt1->value->pA[pRel] == NULL)
						{
							// inserisco nell'albero delle entità con le qauali ho stretto amicizia
							inserisciInListaRelazioni(&punt1->value->pA[pRel], punt2->value);
							punt1->value->nElementiA[pRel]++;     // incremento il numero di elementi della relazione nellentità che riceve l'amicizia
						}
						else
						{
							if (riusaNodo == true)					// se il nodo è già presente nell'albero lo riuso ed aggiorno la versione
								puntRel->versione = punt2->value->versione;
							else
							{
								inserisciInListaRelazioni(&punt1->value->pA[pRel], punt2->value);
								punt1->value->nElementiA[pRel]++;     // incremento il numero di elementi della relazione nellentità che riceve l'amicizia
							}
						}



						// ricalcolo delle entità con il massimo numero di associati
						if (punt2->value->nElementiR[pRel] == maxRelazioni->nElementiA[pRel])
						{
							// aggiungo l'entità all'albero delle entità che hanno il massimo numero di entità associate
							inserisciInListaRelazioni(&maxRelazioni->pA[pRel], punt2->value);
							numNodiMaxRel[pRel]++;
						}
						else
						{
							// la nuova relazione fa si che si sia trovata una nuova entità che a un numero 
							// di elementi associati maggiori di quelli precedentemente trovati
							if (punt2->value->nElementiR[pRel] > maxRelazioni->nElementiA[pRel])
							{
								if (maxRelazioni->pA[pRel] != NULL)
								{
									// elimino per la relazione l'albero non più aggiornato 
									rimuoviTutteLeRelazioni((tree)maxRelazioni->pA[pRel]);
									maxRelazioni->pA[pRel] = NULL;
								}

								// genero un nuovo albero contente le entità con il massimo numero di associati
								inserisciInListaRelazioni(&maxRelazioni->pA[pRel], punt2->value);
								maxRelazioni->nElementiA[pRel] = punt2->value->nElementiR[pRel];     
								numNodiMaxRel[pRel] = 1;
							}
						}
					}
				}
			}
			break;
		case DELREL:		// Cancellazione di un associazione
			if (punt1 != NULL && punt2 != NULL)
			{

				if (punt1->value->attivo == true && punt2->value->attivo == true)
				{
					pRel = posizioneRelazione(p3, vetRelazioni, &numRelazioni);

					Node *puntRel = NULL;
					// verifico che la relazione esista prima di cancellarla
					if (punt1->value->pA[pRel] != NULL)
						puntRel = cercaInListaRelazioni((tree)punt1->value->pA[pRel], punt2->value);

					if (puntRel != NULL)
					{
						if (puntRel->versione == punt2->value->versione)  // se la versione dell'associazione coincide con quella dell'entità decremento
						{
							numEntitaRel = punt2->value->nElementiR[pRel];
							if (numEntitaRel > 0)
							{
								// diminisco di uno il numero di amici dell'entita
								punt2->value->nElementiR[pRel]--;
								// elimino dall entità che aveva stabilito la relazione il riferimento all'entità ricevenete
								rimuoviNodoListaRelazioni(&punt1->value->pA[pRel], puntRel);
								
								punt1->value->nElementiA[pRel]--;     // decremento il numero di elementi associati 

								if (numEntitaRel == maxRelazioni->nElementiA[pRel])
								{
									numNodiMaxRel[pRel]--;
									if (numNodiMaxRel[pRel] == 0)
									{
										// devo scorrere l'albero per ricreare la lista contenete gli elementi che hanno 
										// il massimo numero di elementi associati per la relazione in questione
										
										rimuoviTutteLeRelazioni((tree)maxRelazioni->pA[pRel]);
										maxRelazioni->pA[pRel]=NULL;
										maxElementiRel = 0;
										BSTInorderMAX(reteSociale, pRel, &maxElementiRel);
										maxRelazioni->nElementiA[pRel] = 0;
										numNodiMaxRel[pRel] = 0;
										if (maxElementiRel > 0)
											aggiornaListaEntitaConMaxElementi(reteSociale, pRel, maxElementiRel, maxRelazioni, numNodiMaxRel); // FORSE è meglio non farlo e farlo solo quando viene richiesto il report
									}
									else
									{
										Node *p;
										if (maxRelazioni->pA[pRel] != NULL)
										{
											// controllo che il nodo da eliminare sia presente
											p = cercaInListaRelazioni((tree)maxRelazioni->pA[pRel], punt2->value);
											if (p != NULL)
											{
												rimuoviNodoListaRelazioni(&maxRelazioni->pA[pRel], p);
											}
										}

									}
								}
							}
						}

					}
				}
				break;
		case REPORT:  // stampo il report
			// per stampare le relazioni in ordine alfabetico produco un vettore contenete gli indici delle relazioni ordinate alfabeticamente
			if (nomiOrdinati == false && numRelazioni > 0)
			{
				oldNumeroRelazioni = numRelazioni;
				for (i = 0; i < numRelazioni; i++)
				{
					indiciOrdinati[i] = i;
				}
				indiciRelazioniOrdinateAlfabeticamenteInsertionSort(vetRelazioni, indiciOrdinati, numRelazioni);
				nomiOrdinati = true;
			}
			else  // il vettore è già stato ordinato una volta
			{
				// se sono state inserite nuove relazioni riordino il vettore
				if (oldNumeroRelazioni < numRelazioni)
				{
					oldNumeroRelazioni = numRelazioni;
					for (i = 0; i < numRelazioni; i++)
					{
						indiciOrdinati[i] = i;
					}
					indiciRelazioniOrdinateAlfabeticamenteInsertionSort(vetRelazioni, indiciOrdinati, numRelazioni);
				}
			}
			stampati = 0;  // serve per vedere se non vi sono relazioni da stampare e quindi stampare none
			for (i = 0; i < numRelazioni; i++)
			{
				indice = indiciOrdinati[i];
				//if (maxRelazioni->nElementiR[indice] > 0)
				if (numNodiMaxRel[indice] > 0)
				{
					stampati++;

					// esploro l'entità fittizia utilizzata per contenere le liste dei nomi delle entità con il massimo numero di relazioni e produco l'output
					L = maxRelazioni->pA[indice];
					if (L != NULL)
					{
						fputs(&vetRelazioni[indice][0], stdout);
						fputs(" ", stdout);
						stampaOrdinatElementiRelazione(L);
						printf("%d; ", maxRelazioni->nElementiA[indice]);
					}
				}
			}
			if (stampati == 0)
				printf("none\n");
			else
				printf("\n");

			break;
		case END:

			esci = true;
			}
		}
	}
	deallocaEntita(maxRelazioni);
	liberaMemoria(reteSociale);		// dealloco le strutture dati dinamiche utilizzate
	
}