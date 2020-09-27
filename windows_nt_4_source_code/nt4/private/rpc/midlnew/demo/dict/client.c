/*
 *************************************************************************
 * Remote dictionary example: client side                                *
 *                                                                       *
 * Created:                                 Dov Harel   12/??/1990       *
 * Modified to use context_handle           Donna Liu    3/??/1991       *
 * Further modifications / documentation    Dov Harel    5/1/1991        *
 *                                                                       *
 * Description:                                                          *
 * This is the driver for the client side remote dictionary              *
 * (splay trees based) demo.  This works as follows:                     *
 *                                                                       *
 *  o in main we bind to the dict interface, and call main_dict;         *
 *                                                                       *
 *  o in dict_main we call VDict_New to initialize the dictionary        *
 *    on the server side, and then call TestLoop.                        *
 *                                                                       *
 *  o in TestLoop we loop responding to user input of the form:          *
 *    <one_letter_opcode> [<optional_arg>],                              *
 *    where <optional_arg> is a an integer constant followed by a        *
 *    string.  Type "?" for a detailed usage message.                    *
 *                                                                       *
 *************************************************************************
*/

#define INCL_DOS

#ifdef NTENV
#include <ntos2.h>
#include <ntrtl.h>
#define printf DbgPrint

#else /* NTENV */
#include <os2def.h>
#include <bse.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#endif /* NTENV */

#include <string.h>
#include <rpc.h>
#include "dict0.h"
#include "replay.h"
#include "util0.h"

#define TAB_STOPS 4

#define RDICT_CURR_RECORD(dict) (((DictState*)dict->state)->curr_record)

handle_t dict_bhandle;

char * view = "normal";

void Usage()
{
  printf("Usage : client [-s] [-a<named_pipe_spec>] [-v<view_option>]\n");
  exit(1);
}

/*************************************************************************/
/***                     Remote Dictionary Print                       ***/
/*************************************************************************/

extern char spaces[];

void
RevPrinTree(int lmargin,     /* indentation of the root of the tree     */
    int indent,              /* indentation of subsequent levels        */
    TreeNode *np,            /* pointer to the root node                */
    PrintFun print)          /* short, one line, record print routine   */
{
    int i;
static char lspaces[] =
"                                                                                                                                                                                                                                                       ";
    if (np == NULL) return;

        // prinTree(lmargin+indent, indent, np->right, print);
        RevPrinTree(lmargin+indent, indent, np->left, print);

    if (lmargin > sizeof(lspaces))
        lmargin = sizeof(lspaces);;

    lspaces[lmargin] = 0;
    printf(lspaces);
    lspaces[lmargin] = ' ';

    (*print)(np->item);

        // prinTree(lmargin+indent, indent, np->left, print);
        RevPrinTree(lmargin+indent, indent, np->right, print);

}

void
LinPrinTree(
    TreeNode *np,           /* pointer to the root node                */
    PrintFun print,         /* short, one line, record print routine   */
    Record * local,         /* local iterator point                    */
    Record * global         /* global iterator point                   */
    )
{
    int i;
    if (np == NULL) return;

    LinPrinTree(np->left, print, local, global);

    if ( comp(np->item, local) == 0 ) {
        if ( comp(np->item, global) == 0 )
            printf(" ==>> ");
        else
            printf("   >> ");
        }
    else if ( comp(np->item, global) == 0 )
            printf(" ==   ");
         else
            printf("      ");

    (*print)(np->item);

    LinPrinTree(np->right, print, local, global);

}

void
Clnt_Dict_Print(
    VDict * pvd,
    int indent,
    Record * local,
    Record * global
    )
{
    RDict DictT = {0, 0};
    RDict *prd = &DictT;

    // first: get a new copy a from the server

    VDict_Get_Dict(*pvd, &prd);

    if ( !strcmp(view, "normal") )
        prinTree(0, TAB_STOPS, (TreeNode *) prd->root, printRecord);
    else if ( !strcmp(view, "reverse") || !strcmp(view, "rev") )
        RevPrinTree(0, TAB_STOPS, (TreeNode *) prd->root, printRecord);
    else if ( !strcmp(view, "flat") )
        LinPrinTree( (TreeNode *) prd->root, printRecord, local,
           RDICT_CURR_RECORD(prd));

    RDict_Free_Dict(prd);
}

/*************************************************************************/
/***                     Remote Dictionary Test Loop                   ***/
/*************************************************************************/

void
Usage_Msg()
{
    printf("Usage: \nType a single character, followed by an optional key as follows:\n\n");
    printf("i <key> :: Insert <key> into dictionary\n");
    printf("d <key> :: Delete <key> from dictionary\n");
    printf("f <key> :: Find <key> in dictionary\n");
    printf("N :: next of current item in dictionary\n");
    printf("P :: previous of current item in dictionary\n");
    printf("n :: Next of local current item in dictionary\n");
    printf("p :: Previous of local current item in dictionary\n");
    printf("h :: Head (first item) of dictionary\n");
    printf("t :: Tail (last item) of dictionary\n");
    printf("? :: Print this message\n");
    printf("q :: Quit\n\n");
    printf("<key> is <integer> <string>");
}

void
TestLoop( VDict * pvd );

void
TestLoop( VDict * pvd )
{
    int  key;
    char currName[80];
    char name[80];
    char op = 0;
    char buffer[80];

    Record r, currRecord;
    Record *pcurrRecord = &currRecord;
    Record *pr = &r;
    Record * pNullRecord = NULL;

    Dict_Status status;
    pcurrRecord->name = currName;
    pr->name = name;

    VDict_Curr_Item(*pvd, &pcurrRecord);
    ItemCopy(pcurrRecord, pr);

    Clnt_Dict_Print(pvd, TAB_STOPS, pcurrRecord, pr);
    Usage_Msg();

    while ( op != 'q' ) {

        printf("\nnext op (i d x f n N p P h t ? q): ");
        gets(buffer);
        op = buffer[0];

        if (op == 'i' || op == 'd' || op == 'f' || op == 'I')
            sscanf(buffer+1, "%d %s", &pr->key, pr->name);

        // printf("%c %d\n\n", (int)op, key);

        switch (op) {
            case 'h':
                // get Head of list (first record);

                status = VDict_Next(*pvd, &pNullRecord);
                if (pNullRecord != NULL) {
                    ItemCopy(pNullRecord, pcurrRecord);
                    freeRecord(pNullRecord);
                    pNullRecord = NULL;
                }
                break;

            case 't':
                // get Tail of list (last record)

                status = VDict_Prev(*pvd, &pNullRecord);
                if (pNullRecord != NULL) {
                    ItemCopy(pNullRecord, pcurrRecord);
                    freeRecord(pNullRecord);
                    pNullRecord = NULL;
                }
                break;

            case 'f':
                // Find <key>
                status = VDict_Find(*pvd, &pr);
                pr = &r;
                break;

            case 'n':
                // get next record (advance private (local) iterator)
                status = VDict_Next(*pvd, &pcurrRecord);
                break;

            case 'p':
                // get previous record (retreat private (local) iterator)
                status = VDict_Prev(*pvd, &pcurrRecord);
                break;

            case 'r':
                // Reset local iterator to global "current item"
                status = VDict_Curr_Item(*pvd, &pcurrRecord);
                break;

            case 'N':
                // get Next record (advance global iterator)
                status = VDict_Curr_Next(*pvd, &pr);
                pr = &r;
                break;

            case 'P':
                // get Previous record (retreat global iterator)
                status = VDict_Curr_Prev(*pvd, &pr);
                pr = &r;
                break;

            case 'i':
                // Insert <key>
                status = VDict_Insert(*pvd, pr);
                break;

            case 'I':
                // Insert (<num'>,"") for all num' s.t. 3 < num' < num
                status = VDict_I_Dict(*pvd, pr->key);
                break;

            case 'd':
                // Delete <key>
                status = VDict_Delete(*pvd, &pr);
                if (status != ITEM_NOT_FOUND && status != EMPTY_DICTIONARY) {
                    pr = &r;
                }
                break;

            case 'x':
                // Delete DICT_CURR_ITEM
                status = VDict_Curr_Delete(*pvd, &pr);
                if (pr == NULL) {
                    pr = &r;
                }
                break;

            case 'X':
                // Empty the dictionary
                status = VDict_X_Dict(*pvd);
                break;

            case '?':
                Usage_Msg();
                break;
        }
        if (op != '?' && op != 'q')
            Clnt_Dict_Print(pvd, TAB_STOPS, pcurrRecord, pr);
    }
}


/*************************************************************************/
/***                             Main Loop                             ***/
/*************************************************************************/

void main_dict (short SharedDict)
{
    VDict v_dict = (VDict)0;
    VDict * pvdict;

    pvdict = &v_dict;

    printf ("getting a new dict\n");
    VDict_New( SharedDict, pvdict );
    printf ("gotten a new dict in main_dict\n");
    TestLoop(pvdict);
}

main(int argc, char *argv[])
{
    int argscan;
    char * InterfaceAddress = NULL;
    // RPC_HANDLE Server, Address, Interface;
    RPC_STATUS status;
    short Shared_Dictionary = 0; // Share an existing dictionary?

    if (argc > 1)
      for (argscan = 1; argscan < argc; argscan++)
        {
          if (argv[argscan][0] == '-')
            {
              switch (argv[argscan][1])
              {
                case 'a':
                case 'A':
                    InterfaceAddress = &(argv[argscan][2]);
                    break;

                case 's':
                case 'S':
                    Shared_Dictionary = 1;
                    break;

                case 'v':
                case 'V':
                    view = &(argv[argscan][2]);
                    break;

                default:
                    Usage();
              }
            }
          else
            Usage();
        }
    if (InterfaceAddress == NULL) {
#ifdef NTENV
        InterfaceAddress = "\\device\\namedpipe\\rpc\\replay";
#else /* NTENV */
        // InterfaceAddress = "\\\\dovh3\\pipe\\rpc\\replay";
        InterfaceAddress = "\\pipe\\rpc\\replay";
#endif /* NTENV */
    }

    // Usage();

    /* The dict_ProtocolStack contains a partially initialized
     * protocol stack for the dict interface.  Fill in the remaining
     * fields:
    */
    dict_ProtocolStack.TransportType = RPC_TRANSPORT_NAMEPIPE;

    /*
     * The TransportInfoLength takes into account space for the
     * terminating zero of the address.
    */
    dict_ProtocolStack.TransportInfoLength =
        strlen(InterfaceAddress) + 1;
    dict_ProtocolStack.TransportInfo = InterfaceAddress;

    status = RpcBindToInterface(
                     &dict_ProtocolStack,
                     &dict_DispatchTable,
                     &dict_bhandle
                     );

    if (status)
        {
        printf("RpcBindToInterface = %u\n",status);
        return(1);
        }

    main_dict(Shared_Dictionary);

    status =  RpcUnbind(dict_bhandle);
    if (status)
        printf("RpcUnbind = %u\n",status);

}
