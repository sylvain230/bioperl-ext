#ifdef _cplusplus
extern "C" {
#endif
#include "proteinsw.h"

# line 5 "proteinsw.c"


  /*****************   C functions  ****************/
  /*             Written using dynamite            */
  /*            Wed Sep  9 13:58:08 1998           */
  /*            email birney@sanger.ac.uk          */
  /* http://www.sanger.ac.uk/Users/birney/dynamite */
  /*************************************************/


  /* Please report any problems or bugs to         */
  /* Ewan Birney, birney@sanger.ac.uk              */


/* basic set of macros to map states to numbers */ 
#define MATCH 0  
#define INSERT 1 
#define DELETE 2 


#define START 0  
#define END 1    


#define ProteinSW_EXPL_MATRIX(this_matrix,i,j,STATE) this_matrix->basematrix->matrix[((j+1)*3)+STATE][i+1]   
#define ProteinSW_EXPL_SPECIAL(matrix,i,j,STATE) matrix->basematrix->specmatrix[STATE][j+1]  
#define ProteinSW_READ_OFF_ERROR -3
 




/* Function:  search_ProteinSW(out,querydb,targetdb,comp,gap,ext)
 *
 * Descrip:    This function makes a database search of ProteinSW
 *
 *
 * Arg:             out [UNKN ] Undocumented argument [Hscore *]
 * Arg:         querydb [UNKN ] Undocumented argument [ProteinDB*]
 * Arg:        targetdb [UNKN ] Undocumented argument [ProteinDB*]
 * Arg:            comp [UNKN ] Undocumented argument [CompMat*]
 * Arg:             gap [UNKN ] Undocumented argument [int]
 * Arg:             ext [UNKN ] Undocumented argument [int]
 *
 * Return [UNKN ]  Undocumented return value [Search_Return_Type]
 *
 */
Search_Return_Type search_ProteinSW(Hscore * out,ProteinDB* querydb,ProteinDB* targetdb ,CompMat* comp,int gap,int ext) 
{
    ComplexSequence* query;  
    ComplexSequence* target;     
    int db_status;   
    int score;   
    int query_pos = 0;   
    int target_pos = 0;  
    DataScore * ds;  


    push_errormsg_stack("Before any actual search in db searching"); 
    query = init_ProteinDB(querydb,&db_status);  
    if( db_status == DB_RETURN_ERROR )   {  
      warn("In searching ProteinSW, got a database reload error on the query [query] database"); 
      return SEARCH_ERROR;   
      }  
    for(;;)  {  


      target_pos = 0;    


      target = init_ProteinDB(targetdb,&db_status);  
      if( db_status == DB_RETURN_ERROR )     {  
        warn("In searching ProteinSW, got a database init error on the target [target] database");   
        return SEARCH_ERROR; 
        }  
      for(;;)    {  


        /* No maximum length - allocated on-the-fly */ 
        score = score_only_ProteinSW(query, target , comp, gap, ext);    
        if( should_store_Hscore(out,score) == TRUE )     {  
          ds = new_DataScore_from_storage(out);  
          if( ds == NULL )   {  
            warn("ProteinSW search had a memory error in allocating a new_DataScore (?a leak somewhere - DataScore is a very small datastructure");  
            return SEARCH_ERROR; 
            }  
          /* Now: add query/target information to the entry */ 
          dataentry_add_ProteinDB(ds->query,query,querydb);  
          dataentry_add_ProteinDB(ds->target,target,targetdb);   
          ds->score = score;     
          add_Hscore(out,ds);    
          } /* end of if storing datascore */ 
        pop_errormsg_stack();    
        push_errormsg_stack("DB searching: just finished [Query Pos: %d] [Target Pos: %d]",query_pos,target_pos);    


         target = reload_ProteinDB(target,targetdb,&db_status);  
        if( db_status == DB_RETURN_ERROR )   {  
          warn("In searching ProteinSW, Reload error on database target, position %d,%d",query_pos,target_pos);  
          return SEARCH_ERROR;   
          }  
        if( db_status == DB_RETURN_END ) 
          break;/* Out of target loop */ 
        target_pos++;    
        } /* end of For all target entries */ 
      close_ProteinDB(target,targetdb);  
       query = reload_ProteinDB(query,querydb,&db_status);   
      if( db_status == DB_RETURN_ERROR)  {  
        warn("In searching ProteinSW, Reload error on database query, position %d,%d",query_pos,target_pos); 
        return SEARCH_ERROR; 
        }  
      if( db_status == DB_RETURN_END)    
        break;  /* Out of query loop */ 
      query_pos++;   
      } /* end of For all query entries */ 
    close_ProteinDB(query,querydb);  
    pop_errormsg_stack();    
    return SEARCH_OK;    
}    


#define ProteinSW_VSMALL_MATRIX(mat,i,j,STATE) mat->basematrix->matrix[(j+2)%2][((i+1)*3)+STATE] 
#define ProteinSW_VSMALL_SPECIAL(mat,i,j,STATE) mat->basematrix->specmatrix[(j+2)%2][STATE]  


/* Function:  score_only_ProteinSW(query,target,comp,gap,ext)
 *
 * Descrip:    This function just calculates the score for the matrix
 *             I am pretty sure we can do this better, but hey, for the moment...
 *             It calls /allocate_ProteinSW_only
 *
 *
 * Arg:         query [UNKN ] query data structure [ComplexSequence*]
 * Arg:        target [UNKN ] target data structure [ComplexSequence*]
 * Arg:          comp [UNKN ] Resource [CompMat*]
 * Arg:           gap [UNKN ] Resource [int]
 * Arg:           ext [UNKN ] Resource [int]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int score_only_ProteinSW(ComplexSequence* query,ComplexSequence* target ,CompMat* comp,int gap,int ext) 
{
    int bestscore = NEGI;    
    int i;   
    int j;   
    int k;   
    ProteinSW * mat;     


    mat = allocate_ProteinSW_only(query, target , comp, gap, ext);   
    if( mat == NULL )    {  
      warn("Memory allocation error in the db search - unable to communicate to calling function. this spells DIASTER!");    
      return NEGI;   
      }  
    if((mat->basematrix = BaseMatrix_alloc_matrix_and_specials(2,(mat->leni + 1) * 3,2,2)) == NULL)  {  
      warn("Score only matrix for ProteinSW cannot be allocated, (asking for 1  by %d  cells)",mat->leni*3); 
      mat = free_ProteinSW(mat);     
      return 0;  
      }  
    mat->basematrix->type = BASEMATRIX_TYPE_VERYSMALL;   


    /* Now, initiate matrix */ 
    for(j=0;j<3;j++) {  
      for(i=(-1);i<mat->leni;i++)    {  
        for(k=0;k<3;k++) 
          ProteinSW_VSMALL_MATRIX(mat,i,j,k) = NEGI; 
        }  
      ProteinSW_VSMALL_SPECIAL(mat,i,j,START) = 0;   
      ProteinSW_VSMALL_SPECIAL(mat,i,j,END) = NEGI;  
      }  


    /* Ok, lets do-o-o-o-o it */ 


    for(j=0;j<mat->lenj;j++) {  
      auto int score;    
      auto int temp;     
      for(i=0;i<mat->leni;i++)   {  


        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_VSMALL_MATRIX(mat,i-1,j-1,MATCH) + 0;  
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_VSMALL_MATRIX(mat,i-1,j-1,INSERT) + 0;  
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_VSMALL_MATRIX(mat,i-1,j-1,DELETE) + 0;  
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state START to state MATCH */ 
        temp = ProteinSW_VSMALL_SPECIAL(mat,i-1,j-1,START) + 0;  
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
         score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));     
         ProteinSW_VSMALL_MATRIX(mat,i,j,MATCH) = score; 


        /* state MATCH is a source for special END */ 
        temp = score + (0) + (0) ;   
        if( temp > ProteinSW_VSMALL_SPECIAL(mat,i,j,END) )   {  
          ProteinSW_VSMALL_SPECIAL(mat,i,j,END) = temp;  
          }  




        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_VSMALL_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;   
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_VSMALL_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;   
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_VSMALL_MATRIX(mat,i,j,INSERT) = score;    


        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_VSMALL_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;   
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_VSMALL_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;   
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_VSMALL_MATRIX(mat,i,j,DELETE) = score;    


        /* Finished calculating state DELETE */ 
        } /* end of for all query positions */ 




      /* Special state START has no special to special movements */ 


      /* Special state END has no special to special movements */ 
      if( bestscore < ProteinSW_VSMALL_SPECIAL(mat,0,j,END) )    
        bestscore = ProteinSW_VSMALL_SPECIAL(mat,0,j,END);   
      } /* end of for all target positions */ 


    mat = free_ProteinSW(mat);   
    return bestscore;    
}    


/* Function:  PackAln_bestmemory_ProteinSW(query,target,comp,gap,ext,dpenv)
 *
 * Descrip:    This function chooses the best memory set-up for the alignment
 *             using calls to basematrix, and then implements either a large
 *             or small memory model.
 *
 *             It is the best function to use if you just want an alignment
 *
 *             If you want a label alignment, you will need
 *             /convert_PackAln_to_AlnBlock_ProteinSW
 *
 *
 * Arg:         query [UNKN ] query data structure [ComplexSequence*]
 * Arg:        target [UNKN ] target data structure [ComplexSequence*]
 * Arg:          comp [UNKN ] Resource [CompMat*]
 * Arg:           gap [UNKN ] Resource [int]
 * Arg:           ext [UNKN ] Resource [int]
 * Arg:         dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 * Return [UNKN ]  Undocumented return value [PackAln *]
 *
 */
PackAln * PackAln_bestmemory_ProteinSW(ComplexSequence* query,ComplexSequence* target ,CompMat* comp,int gap,int ext,DPEnvelope * dpenv) 
{
    int total;   
    ProteinSW * mat; 
    PackAln * out;   


    total = query->seq->len * target->seq->len;  


    if( dpenv != NULL || (total * 3 * sizeof(int)) > 1000*get_max_BaseMatrix_kbytes() )  {  
      /* use small implementation */ 
      if( (mat=allocate_Small_ProteinSW(query, target , comp, gap, ext)) == NULL )   {  
        warn("Unable to allocate small ProteinSW version");  
        return NULL; 
        }  
      out = PackAln_calculate_Small_ProteinSW(mat,dpenv);    
      }  
    else {  
      /* use Large implementation */ 
      if( (mat=allocate_Expl_ProteinSW(query, target , comp, gap, ext)) == NULL )    {  
        warn("Unable to allocate large ProteinSW version");  
        return NULL; 
        }  
      calculate_ProteinSW(mat);  
      out =  PackAln_read_Expl_ProteinSW(mat);   
      }  


    mat = free_ProteinSW(mat);   
    return out;  
}    


/* Function:  allocate_ProteinSW_only(query,target,comp,gap,ext)
 *
 * Descrip:    This function only allocates the ProteinSW structure
 *             checks types where possible and determines leni and lenj
 *             The basematrix area is delt with elsewhere
 *
 *
 * Arg:         query [UNKN ] query data structure [ComplexSequence*]
 * Arg:        target [UNKN ] target data structure [ComplexSequence*]
 * Arg:          comp [UNKN ] Resource [CompMat*]
 * Arg:           gap [UNKN ] Resource [int]
 * Arg:           ext [UNKN ] Resource [int]
 *
 * Return [UNKN ]  Undocumented return value [ProteinSW *]
 *
 */
ProteinSW * allocate_ProteinSW_only(ComplexSequence* query,ComplexSequence* target ,CompMat* comp,int gap,int ext) 
{
    ProteinSW * out;     


    if((out= ProteinSW_alloc()) == NULL) {  
      warn("Allocation of basic ProteinSW structure failed..."); 
      return NULL;   
      }  


    out->query = query;  
    out->target = target;    
    out->comp = comp;    
    out->gap = gap;  
    out->ext = ext;  
    out->leni = query->seq->len;     
    out->lenj = target->seq->len;    
    return out;  
}    


/* Function:  allocate_Expl_ProteinSW(query,target,comp,gap,ext)
 *
 * Descrip:    This function allocates the ProteinSW structure
 *             and the basematrix area for explicit memory implementations
 *             It calls /allocate_ProteinSW_only
 *
 *
 * Arg:         query [UNKN ] query data structure [ComplexSequence*]
 * Arg:        target [UNKN ] target data structure [ComplexSequence*]
 * Arg:          comp [UNKN ] Resource [CompMat*]
 * Arg:           gap [UNKN ] Resource [int]
 * Arg:           ext [UNKN ] Resource [int]
 *
 * Return [UNKN ]  Undocumented return value [ProteinSW *]
 *
 */
ProteinSW * allocate_Expl_ProteinSW(ComplexSequence* query,ComplexSequence* target ,CompMat* comp,int gap,int ext) 
{
    ProteinSW * out; 


    out = allocate_ProteinSW_only(query, target , comp, gap, ext);   
    if( out == NULL )    
      return NULL;   
    if( (out->basematrix = BaseMatrix_alloc_matrix_and_specials((out->lenj+1)*3,(out->leni+1),2,out->lenj+1)) == NULL)   {  
      warn("Explicit matrix ProteinSW cannot be allocated, (asking for %d by %d main cells)",out->leni,out->lenj);   
      free_ProteinSW(out);   
      return NULL;   
      }  
    out->basematrix->type = BASEMATRIX_TYPE_EXPLICIT;    
    init_ProteinSW(out);     
    return out;  
}    


/* Function:  init_ProteinSW(mat)
 *
 * Descrip:    This function initates ProteinSW matrix when in explicit mode
 *             Called in /allocate_Expl_ProteinSW
 *
 *
 * Arg:        mat [UNKN ] ProteinSW which contains explicit basematrix memory [ProteinSW *]
 *
 */
void init_ProteinSW(ProteinSW * mat) 
{
    register int i;  
    register int j;  
    if( mat->basematrix->type != BASEMATRIX_TYPE_EXPLICIT)   {  
      warn("Cannot iniate matrix, is not an explicit memory type and you have assummed that");   
      return;    
      }  


    for(i= (-1);i<mat->query->seq->len;i++)  {  
      for(j= (-1);j<2;j++)   {  
        ProteinSW_EXPL_MATRIX(mat,i,j,MATCH) = NEGI; 
        ProteinSW_EXPL_MATRIX(mat,i,j,INSERT) = NEGI;    
        ProteinSW_EXPL_MATRIX(mat,i,j,DELETE) = NEGI;    
        }  
      }  
    for(j= (-1);j<mat->target->seq->len;j++) {  
      for(i= (-1);i<2;i++)   {  
        ProteinSW_EXPL_MATRIX(mat,i,j,MATCH) = NEGI; 
        ProteinSW_EXPL_MATRIX(mat,i,j,INSERT) = NEGI;    
        ProteinSW_EXPL_MATRIX(mat,i,j,DELETE) = NEGI;    
        }  
      ProteinSW_EXPL_SPECIAL(mat,i,j,START) = 0; 
      ProteinSW_EXPL_SPECIAL(mat,i,j,END) = NEGI;    
      }  
    return;  
}    


/* Function:  recalculate_PackAln_ProteinSW(pal,mat)
 *
 * Descrip:    This function recalculates the PackAln structure produced by ProteinSW
 *             For example, in linear space methods this is used to score them
 *
 *
 * Arg:        pal [UNKN ] Undocumented argument [PackAln *]
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 */
void recalculate_PackAln_ProteinSW(PackAln * pal,ProteinSW * mat) 
{
    int i,j,k,offi,offj; 
    PackAlnUnit * prev;  
    PackAlnUnit * pau;   


    for(k=1,prev=pal->pau[0];k < pal->len;k++,prev=pau)  {  
      pau = pal->pau[k]; 
      i = pau->i;    
      j = pau->j;    
      offi = pau->i - prev->i;   
      offj = pau->j - prev->j;   
      switch(pau->state) {  
        case MATCH :     
          if( offi == 1 && offj == 1 && prev->state == MATCH )   {  
            pau->score = 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));    
            continue;    
            }  
          if( offi == 1 && offj == 1 && prev->state == INSERT )  {  
            pau->score = 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));    
            continue;    
            }  
          if( offi == 1 && offj == 1 && prev->state == DELETE )  {  
            pau->score = 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));    
            continue;    
            }  
          if( offj == 1 && prev->state == (START+3) )    {  
            pau->score = 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));    
            continue;    
            }  
          warn("In recaluclating PackAln with state MATCH, from [%d,%d,%d], got a bad source state. Error!",offi,offj,prev->state);  
          break; 
        case INSERT :    
          if( offi == 0 && offj == 1 && prev->state == MATCH )   {  
            pau->score = mat->gap + (0);     
            continue;    
            }  
          if( offi == 0 && offj == 1 && prev->state == INSERT )  {  
            pau->score = mat->ext + (0);     
            continue;    
            }  
          warn("In recaluclating PackAln with state INSERT, from [%d,%d,%d], got a bad source state. Error!",offi,offj,prev->state); 
          break; 
        case DELETE :    
          if( offi == 1 && offj == 0 && prev->state == MATCH )   {  
            pau->score = mat->gap + (0);     
            continue;    
            }  
          if( offi == 1 && offj == 0 && prev->state == DELETE )  {  
            pau->score = mat->ext + (0);     
            continue;    
            }  
          warn("In recaluclating PackAln with state DELETE, from [%d,%d,%d], got a bad source state. Error!",offi,offj,prev->state); 
          break; 
        case (START+3) :     
          warn("In recaluclating PackAln with state START, got a bad source state. Error!"); 
          break; 
        case (END+3) :   
          if( offj == 0 && prev->state == MATCH )    {  
            /* i here comes from the previous state ;) - not the real one */ 
            i = prev->i; 
            pau->score = 0 + (0);    
            continue;    
            }  
          warn("In recaluclating PackAln with state END, got a bad source state. Error!");   
          break; 
        default :    
          warn("In recaluclating PackAln got a bad recipient state. Error!");    
        }  
      prev = pau;    
      }  
    return;  
}    
/* divide and conquor macros are next */ 
#define ProteinSW_HIDDEN_MATRIX(thismatrix,i,j,state) (thismatrix->basematrix->matrix[(j-hiddenj+1)][(i+1)*3+state]) 
#define ProteinSW_DC_SHADOW_MATRIX(thismatrix,i,j,state) (thismatrix->basematrix->matrix[((j+2)*8) % 16][(i+1)*3+state]) 
#define ProteinSW_HIDDEN_SPECIAL(thismatrix,i,j,state) (thismatrix->basematrix->specmatrix[state][(j+1)])    
#define ProteinSW_DC_SHADOW_SPECIAL(thismatrix,i,j,state) (thismatrix->basematrix->specmatrix[state*8][(j+1)])   
#define ProteinSW_DC_SHADOW_MATRIX_SP(thismatrix,i,j,state,shadow) (thismatrix->basematrix->matrix[((((j+2)*8)+(shadow+1)) % 16)][(i+1)*3 + state])  
/* Function:  allocate_Small_ProteinSW(query,target,comp,gap,ext)
 *
 * Descrip:    This function allocates the ProteinSW structure
 *             and the basematrix area for a small memory implementations
 *             It calls /allocate_ProteinSW_only
 *
 *
 * Arg:         query [UNKN ] query data structure [ComplexSequence*]
 * Arg:        target [UNKN ] target data structure [ComplexSequence*]
 * Arg:          comp [UNKN ] Resource [CompMat*]
 * Arg:           gap [UNKN ] Resource [int]
 * Arg:           ext [UNKN ] Resource [int]
 *
 * Return [UNKN ]  Undocumented return value [ProteinSW *]
 *
 */
#define ProteinSW_DC_SHADOW_SPECIAL_SP(thismatrix,i,j,state,shadow) (thismatrix->basematrix->specmatrix[state*8 +shadow+1][(j+1)])   
ProteinSW * allocate_Small_ProteinSW(ComplexSequence* query,ComplexSequence* target ,CompMat* comp,int gap,int ext) 
{
    ProteinSW * out; 


    out = allocate_ProteinSW_only(query, target , comp, gap, ext);   
    if( out == NULL )    
      return NULL;   
    out->basematrix = BaseMatrix_alloc_matrix_and_specials(16,(out->leni + 1) * 3,16,out->lenj+1);   
    if(out == NULL)  {  
      warn("Small shadow matrix ProteinSW cannot be allocated, (asking for 2 by %d main cells)",out->leni+2);    
      free_ProteinSW(out);   
      return NULL;   
      }  
    out->basematrix->type = BASEMATRIX_TYPE_SHADOW;  
    return out;  
}    


/* Function:  PackAln_calculate_Small_ProteinSW(mat,dpenv)
 *
 * Descrip:    This function calculates an alignment for ProteinSW structure in linear space
 *             If you want only the start/end points
 *             use /AlnRangeSet_calculate_Small_ProteinSW 
 *
 *             The function basically
 *               finds start/end points 
 *               foreach start/end point 
 *                 calls /full_dc_ProteinSW 
 *
 *
 * Arg:          mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:        dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 * Return [UNKN ]  Undocumented return value [PackAln *]
 *
 */
PackAln * PackAln_calculate_Small_ProteinSW(ProteinSW * mat,DPEnvelope * dpenv) 
{
    int endj;    
    int score;   
    PackAln * out;   
    PackAlnUnit * pau;   
    int starti;  
    int startj;  
    int startstate;  
    int stopi;   
    int stopj;   
    int stopstate;   
    int temp;    
    int donej;  /* This is for reporting, will be passed as a & arg in */ 
    int totalj; /* This also is for reporting, but as is not changed, can be passed by value */ 


    if( mat->basematrix->type != BASEMATRIX_TYPE_SHADOW )    {  
      warn("Could not calculate packaln small for ProteinSW due to wrong type of matrix");   
      return NULL;   
      }  


    out = PackAln_alloc_std();   


    start_reporting("Find start end points: ");  
    dc_start_end_calculate_ProteinSW(mat,dpenv); 
    score = start_end_find_end_ProteinSW(mat,&endj); 
    out->score = score;  
    stopstate = END;
    
    /* No special to specials: one matrix alignment: simply remove and get */ 
    starti = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,0);   
    startj = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,1);   
    startstate = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,2);   
    stopi = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,3);    
    stopj = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,4);    
    stopstate = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,5);    
    temp = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,endj,END,6); 
    log_full_error(REPORT,0,"[%d,%d][%d,%d] Score %d",starti,startj,stopi,stopj,score);  
    stop_reporting();    
    start_reporting("Recovering alignment: ");   


    /* Figuring how much j we have to align for reporting purposes */ 
    donej = 0;   
    totalj = stopj - startj; 
    full_dc_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,out,&donej,totalj,dpenv);   


    /* Although we have no specials, need to get start. Better to check than assume */ 


    max_matrix_to_special_ProteinSW(mat,starti,startj,startstate,temp,&stopi,&stopj,&stopstate,&temp,NULL);  
    if( stopi == ProteinSW_READ_OFF_ERROR || stopstate != START )    {  
      warn("Problem in reading off special state system, hit a non start state (or an internal error) in a single alignment mode");  
      invert_PackAln(out);   
      recalculate_PackAln_ProteinSW(out,mat);    
      return out;    
      }  


    /* Ok. Put away start start... */ 
    pau = PackAlnUnit_alloc();   
    pau->i = stopi;  
    pau->j = stopj;  
    pau->state = stopstate + 3;  
    add_PackAln(out,pau);    


    log_full_error(REPORT,0,"Alignment recovered");  
    stop_reporting();    
    invert_PackAln(out); 
    recalculate_PackAln_ProteinSW(out,mat);  
    return out;  


}    


/* Function:  AlnRangeSet_calculate_Small_ProteinSW(mat)
 *
 * Descrip:    This function calculates an alignment for ProteinSW structure in linear space
 *             If you want the full alignment, use /PackAln_calculate_Small_ProteinSW 
 *             If you have already got the full alignment, but want the range set, use /AlnRangeSet_from_PackAln_ProteinSW
 *             If you have got the small matrix but not the alignment, use /AlnRangeSet_from_ProteinSW 
 *
 *
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 * Return [UNKN ]  Undocumented return value [AlnRangeSet *]
 *
 */
AlnRangeSet * AlnRangeSet_calculate_Small_ProteinSW(ProteinSW * mat) 
{
    AlnRangeSet * out;   


    start_reporting("Find start end points: ");  
    dc_start_end_calculate_ProteinSW(mat,NULL);  
    log_full_error(REPORT,0,"Calculated");   


    out = AlnRangeSet_from_ProteinSW(mat);   
    return out;  
}    


/* Function:  AlnRangeSet_from_ProteinSW(mat)
 *
 * Descrip:    This function reads off a start/end structure
 *             for ProteinSW structure in linear space
 *             If you want the full alignment use
 *             /PackAln_calculate_Small_ProteinSW 
 *             If you have not calculated the matrix use
 *             /AlnRange_calculate_Small_ProteinSW
 *
 *
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 * Return [UNKN ]  Undocumented return value [AlnRangeSet *]
 *
 */
AlnRangeSet * AlnRangeSet_from_ProteinSW(ProteinSW * mat) 
{
    AlnRangeSet * out;   
    AlnRange * temp; 
    int jpos;    
    int state;   


    if( mat->basematrix->type != BASEMATRIX_TYPE_SHADOW) {  
      warn("Bad error! - non shadow matrix type in AlnRangeSet_from_ProteinSW"); 
      return NULL;   
      }  


    out = AlnRangeSet_alloc_std();   
    /* Find the end position */ 
    out->score = start_end_find_end_ProteinSW(mat,&jpos);    
    state = END; 


    while( (temp = AlnRange_build_ProteinSW(mat,jpos,state,&jpos,&state)) != NULL)   
      add_AlnRangeSet(out,temp); 
    return out;  
}    


/* Function:  AlnRange_build_ProteinSW(mat,stopj,stopspecstate,startj,startspecstate)
 *
 * Descrip:    This function calculates a single start/end set in linear space
 *             Really a sub-routine for /AlnRangeSet_from_PackAln_ProteinSW
 *
 *
 * Arg:                   mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:                 stopj [UNKN ] Undocumented argument [int]
 * Arg:         stopspecstate [UNKN ] Undocumented argument [int]
 * Arg:                startj [UNKN ] Undocumented argument [int *]
 * Arg:        startspecstate [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [AlnRange *]
 *
 */
AlnRange * AlnRange_build_ProteinSW(ProteinSW * mat,int stopj,int stopspecstate,int * startj,int * startspecstate) 
{
    AlnRange * out;  
    int jpos;    
    int state;   


    if( mat->basematrix->type != BASEMATRIX_TYPE_SHADOW) {  
      warn("Bad error! - non shadow matrix type in AlnRangeSet_from_ProteinSW"); 
      return NULL;   
      }  


    /* Assumme that we have specials (we should!). Read back along the specials till we have the finish point */ 
    if( read_special_strip_ProteinSW(mat,0,stopj,stopspecstate,&jpos,&state,NULL) == FALSE)  {  
      warn("In AlnRanger_build_ProteinSW alignment ending at %d, unable to read back specials. Will (evenutally) return a partial range set... BEWARE!",stopj);  
      return NULL;   
      }  
    if( state == START || jpos <= 0) 
      return NULL;   


    out = AlnRange_alloc();  


    out->starti = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,0);    
    out->startj = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,1);    
    out->startstate = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,2);    
    out->stopi = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,3); 
    out->stopj = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,4); 
    out->stopstate = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,5); 
    out->startscore = ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,jpos,state,6);    
    out->stopscore = ProteinSW_DC_SHADOW_SPECIAL(mat,0,jpos,state);  


    /* Now, we have to figure out where this state came from in the specials */ 
    max_matrix_to_special_ProteinSW(mat,out->starti,out->startj,out->startstate,out->startscore,&jpos,startj,startspecstate,&state,NULL);    
    if( jpos == ProteinSW_READ_OFF_ERROR)    {  
      warn("In AlnRange_build_ProteinSW alignment ending at %d, with aln range between %d-%d in j, unable to find source special, returning this range, but this could get tricky!",stopj,out->startj,out->stopj);   
      return out;    
      }  


    /* Put in the correct score for startstate, from the special */ 
    out->startscore = ProteinSW_DC_SHADOW_SPECIAL(mat,0,*startj,*startspecstate);    
    /* The correct j coords have been put into startj, startspecstate... so just return out */ 
    return out;  
}    


/* Function:  read_hidden_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,out)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:            starti [UNKN ] Undocumented argument [int]
 * Arg:            startj [UNKN ] Undocumented argument [int]
 * Arg:        startstate [UNKN ] Undocumented argument [int]
 * Arg:             stopi [UNKN ] Undocumented argument [int]
 * Arg:             stopj [UNKN ] Undocumented argument [int]
 * Arg:         stopstate [UNKN ] Undocumented argument [int]
 * Arg:               out [UNKN ] Undocumented argument [PackAln *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean read_hidden_ProteinSW(ProteinSW * mat,int starti,int startj,int startstate,int stopi,int stopj,int stopstate,PackAln * out) 
{
    int i;   
    int j;   
    int state;   
    int cellscore;   
    int isspecial;   
    /* We don't need hiddenj here, 'cause matrix access handled by max funcs */ 
    PackAlnUnit * pau;   


    /* stop position is on the path */ 
    i = stopi;   
    j = stopj;   
    state= stopstate;    
    isspecial = FALSE;   


    while( i >= starti && j >= startj)   {  
      /* Put away current i,j,state */ 
      pau = PackAlnUnit_alloc();/* Should deal with memory overflow */ 
      pau->i = i;    
      pau->j = j;    
      pau->state =  state;   
      add_PackAln(out,pau);  


      max_hidden_ProteinSW(mat,startj,i,j,state,isspecial,&i,&j,&state,&isspecial,&cellscore);   


      if( i == ProteinSW_READ_OFF_ERROR) {  
        warn("In ProteinSW hidden read off, between %d:%d,%d:%d - at got bad read off. Problem!",starti,startj,stopi,stopj); 
        return FALSE;    
        }  


      if( i == starti && j == startj && state == startstate) {  
/* Put away final state (start of this block) */ 
        pau = PackAlnUnit_alloc();  /* Should deal with memory overflow */ 
        pau->i = i;  
        pau->j = j;  
        pau->state =  state; 
        add_PackAln(out,pau);    
          return TRUE;   
        }  
      if( i == starti && j == startj)    {  
        warn("In ProteinSW hidden read off, between %d:%d,%d:%d - hit start cell, but not in start state. Can't be good!.",starti,startj,stopi,stopj);   
        return FALSE;    
        }  
      }  
    warn("In ProteinSW hidden read off, between %d:%d,%d:%d - gone past start cell (now in %d,%d,%d), can't be good news!.",starti,startj,stopi,stopj,i,j,state);    
    return FALSE;    
}    


/* Function:  max_hidden_ProteinSW(mat,hiddenj,i,j,state,isspecial,reti,retj,retstate,retspecial,cellscore)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:           hiddenj [UNKN ] Undocumented argument [int]
 * Arg:                 i [UNKN ] Undocumented argument [int]
 * Arg:                 j [UNKN ] Undocumented argument [int]
 * Arg:             state [UNKN ] Undocumented argument [int]
 * Arg:         isspecial [UNKN ] Undocumented argument [boolean]
 * Arg:              reti [UNKN ] Undocumented argument [int *]
 * Arg:              retj [UNKN ] Undocumented argument [int *]
 * Arg:          retstate [UNKN ] Undocumented argument [int *]
 * Arg:        retspecial [UNKN ] Undocumented argument [boolean *]
 * Arg:         cellscore [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int max_hidden_ProteinSW(ProteinSW * mat,int hiddenj,int i,int j,int state,boolean isspecial,int * reti,int * retj,int * retstate,boolean * retspecial,int * cellscore) 
{
    register int temp;   
    register int cscore; 


    *reti = (*retj) = (*retstate) = ProteinSW_READ_OFF_ERROR;    


    if( i < 0 || j < 0 || i > mat->query->seq->len || j > mat->target->seq->len) {  
      warn("In ProteinSW matrix special read off - out of bounds on matrix [i,j is %d,%d state %d in standard matrix]",i,j,state);   
      return -1; 
      }  


    /* Then you have to select the correct switch statement to figure out the readoff      */ 
    /* Somewhat odd - reverse the order of calculation and return as soon as it is correct */ 
    cscore = ProteinSW_HIDDEN_MATRIX(mat,i,j,state); 
    switch(state)    {  
      case MATCH :   
        /* Not allowing special sources.. skipping START */ 
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,DELETE) )    {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = DELETE;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,DELETE);   
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,DELETE);    
          }  
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,INSERT) )    {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = INSERT;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,INSERT);   
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,INSERT);    
          }  
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,MATCH) ) {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,MATCH);    
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 1,MATCH);     
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      case INSERT :  
        temp = cscore - (mat->ext) -  (0);   
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 0,j - 1,INSERT) )    {  
          *reti = i - 0; 
          *retj = j - 1; 
          *retstate = INSERT;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-0,j-1,INSERT);   
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 0,j - 1,INSERT);    
          }  
        temp = cscore - (mat->gap) -  (0);   
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 0,j - 1,MATCH) ) {  
          *reti = i - 0; 
          *retj = j - 1; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-0,j-1,MATCH);    
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 0,j - 1,MATCH);     
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      case DELETE :  
        temp = cscore - (mat->ext) -  (0);   
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 0,DELETE) )    {  
          *reti = i - 1; 
          *retj = j - 0; 
          *retstate = DELETE;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-1,j-0,DELETE);   
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 0,DELETE);    
          }  
        temp = cscore - (mat->gap) -  (0);   
        if( temp == ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 0,MATCH) ) {  
          *reti = i - 1; 
          *retj = j - 0; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_HIDDEN_MATRIX(mat,i-1,j-0,MATCH);    
            }  
          return ProteinSW_HIDDEN_MATRIX(mat,i - 1,j - 0,MATCH);     
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      default:   
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      } /* end of Switch state  */ 
}    


/* Function:  read_special_strip_ProteinSW(mat,stopi,stopj,stopstate,startj,startstate,out)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:             stopi [UNKN ] Undocumented argument [int]
 * Arg:             stopj [UNKN ] Undocumented argument [int]
 * Arg:         stopstate [UNKN ] Undocumented argument [int]
 * Arg:            startj [UNKN ] Undocumented argument [int *]
 * Arg:        startstate [UNKN ] Undocumented argument [int *]
 * Arg:               out [UNKN ] Undocumented argument [PackAln *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean read_special_strip_ProteinSW(ProteinSW * mat,int stopi,int stopj,int stopstate,int * startj,int * startstate,PackAln * out) 
{
    int i;   
    int j;   
    int state;   
    int cellscore;   
    int isspecial;   
    PackAlnUnit * pau;   


    /* stop position is on the path */ 
    i = stopi;   
    j = stopj;   
    state= stopstate;    
    isspecial = TRUE;    


    /* Loop until state has the same j as its stop in shadow pointers */ 
    /* This will be the state is came out from, OR it has hit !start */ 
    /* We may not want to get the alignment, in which case out will be NULL */ 
    while( j > ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,state,4) && state != START)    {  
      /* Put away current state, if we should */ 
      if(out != NULL)    {  
        pau = PackAlnUnit_alloc();  /* Should deal with memory overflow */ 
        pau->i = i;  
        pau->j = j;  
        pau->state =  state + 3; 
        add_PackAln(out,pau);    
        }  


      max_special_strip_ProteinSW(mat,i,j,state,isspecial,&i,&j,&state,&isspecial,&cellscore);   
      if( i == ProteinSW_READ_OFF_ERROR) {  
        warn("In special strip read ProteinSW, got a bad read off error. Sorry!");   
        return FALSE;    
        }  
      } /* end of while more specials to eat up */ 


    /* check to see we have not gone too far! */ 
    if( state != START && j < ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,state,4))   {  
      warn("In special strip read ProteinSW, at special [%d] state [%d] overshot!",j,state); 
      return FALSE;  
      }  
    /* Put away last state */ 
    if(out != NULL)  {  
      pau = PackAlnUnit_alloc();/* Should deal with memory overflow */ 
      pau->i = i;    
      pau->j = j;    
      pau->state =  state + 3;   
      add_PackAln(out,pau);  
      }  


    /* Put away where we are in startj and startstate */ 
    *startj = j; 
    *startstate = state; 
    return TRUE; 
}    


/* Function:  max_special_strip_ProteinSW(mat,i,j,state,isspecial,reti,retj,retstate,retspecial,cellscore)
 *
 * Descrip:    A pretty intense internal function. Deals with read-off only in specials
 *
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:                 i [UNKN ] Undocumented argument [int]
 * Arg:                 j [UNKN ] Undocumented argument [int]
 * Arg:             state [UNKN ] Undocumented argument [int]
 * Arg:         isspecial [UNKN ] Undocumented argument [boolean]
 * Arg:              reti [UNKN ] Undocumented argument [int *]
 * Arg:              retj [UNKN ] Undocumented argument [int *]
 * Arg:          retstate [UNKN ] Undocumented argument [int *]
 * Arg:        retspecial [UNKN ] Undocumented argument [boolean *]
 * Arg:         cellscore [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int max_special_strip_ProteinSW(ProteinSW * mat,int i,int j,int state,boolean isspecial,int * reti,int * retj,int * retstate,boolean * retspecial,int * cellscore) 
{
    int temp;    
    int cscore;  


    *reti = (*retj) = (*retstate) = ProteinSW_READ_OFF_ERROR;    
    if( isspecial == FALSE ) {  
      warn("In special strip max function for ProteinSW, got a non special start point. Problem! (bad!)");   
      return (-1);   
      }  


    if( j < 0 || j > mat->target->seq->len)  {  
      warn("In ProteinSW matrix special read off - out of bounds on matrix [j is %d in special]",j); 
      return -1; 
      }  


    cscore = ProteinSW_DC_SHADOW_SPECIAL(mat,i,j,state); 
    switch(state)    {  
      case START :   
      case END :     
        /* Source MATCH is not a special */ 
      default:   
        warn("Major problem (!) - in ProteinSW special strip read off, position %d,%d state %d no source found  dropped into default on source switch!",i,j,state);  
        return (-1); 
      } /* end of switch on special states */ 
}    


/* Function:  max_matrix_to_special_ProteinSW(mat,i,j,state,cscore,reti,retj,retstate,retspecial,cellscore)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:                 i [UNKN ] Undocumented argument [int]
 * Arg:                 j [UNKN ] Undocumented argument [int]
 * Arg:             state [UNKN ] Undocumented argument [int]
 * Arg:            cscore [UNKN ] Undocumented argument [int]
 * Arg:              reti [UNKN ] Undocumented argument [int *]
 * Arg:              retj [UNKN ] Undocumented argument [int *]
 * Arg:          retstate [UNKN ] Undocumented argument [int *]
 * Arg:        retspecial [UNKN ] Undocumented argument [boolean *]
 * Arg:         cellscore [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int max_matrix_to_special_ProteinSW(ProteinSW * mat,int i,int j,int state,int cscore,int * reti,int * retj,int * retstate,boolean * retspecial,int * cellscore) 
{
    int temp;    
    *reti = (*retj) = (*retstate) = ProteinSW_READ_OFF_ERROR;    


    if( j < 0 || j > mat->lenj)  {  
      warn("In ProteinSW matrix to special read off - out of bounds on matrix [j is %d in special]",j);  
      return -1; 
      }  


    switch(state)    {  
      case MATCH :   
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_DC_SHADOW_SPECIAL(mat,i - 1,j - 1,START) ) {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = START; 
          *retspecial = TRUE;    
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_DC_SHADOW_SPECIAL(mat,i-1,j-1,START);    
            }  
          return ProteinSW_DC_SHADOW_MATRIX(mat,i - 1,j - 1,START) ;     
          }  
        /* Source DELETE is not a special, should not get here! */ 
        /* Source INSERT is not a special, should not get here! */ 
        /* Source MATCH is not a special, should not get here! */ 
        warn("Major problem (!) - in ProteinSW matrix to special read off, position %d,%d state %d no source found!",i,j,state); 
        return (-1); 
      case INSERT :  
        /* Source INSERT is not a special, should not get here! */ 
        /* Source MATCH is not a special, should not get here! */ 
        warn("Major problem (!) - in ProteinSW matrix to special read off, position %d,%d state %d no source found!",i,j,state); 
        return (-1); 
      case DELETE :  
        /* Source DELETE is not a special, should not get here! */ 
        /* Source MATCH is not a special, should not get here! */ 
        warn("Major problem (!) - in ProteinSW matrix to special read off, position %d,%d state %d no source found!",i,j,state); 
        return (-1); 
      default:   
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      } /* end of Switch state  */ 


}    


/* Function:  calculate_hidden_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,dpenv)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:            starti [UNKN ] Undocumented argument [int]
 * Arg:            startj [UNKN ] Undocumented argument [int]
 * Arg:        startstate [UNKN ] Undocumented argument [int]
 * Arg:             stopi [UNKN ] Undocumented argument [int]
 * Arg:             stopj [UNKN ] Undocumented argument [int]
 * Arg:         stopstate [UNKN ] Undocumented argument [int]
 * Arg:             dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 */
void calculate_hidden_ProteinSW(ProteinSW * mat,int starti,int startj,int startstate,int stopi,int stopj,int stopstate,DPEnvelope * dpenv) 
{
    register int i;  
    register int j;  
    register int score;  
    register int temp;   
    register int hiddenj;    


    hiddenj = startj;    


    init_hidden_ProteinSW(mat,starti,startj,stopi,stopj);    


    ProteinSW_HIDDEN_MATRIX(mat,starti,startj,startstate) = 0;   


    for(j=startj;j<=stopj;j++)   {  
      for(i=starti;i<=stopi;i++) {  
        /* Should *not* do very first cell as this is the one set to zero in one state! */ 
        if( i == starti && j == startj ) 
          continue;  
        if( dpenv != NULL && is_in_DPEnvelope(dpenv,i,j) == FALSE )  {  
          ProteinSW_HIDDEN_MATRIX(mat,i,j,MATCH) = NEGI;     
          ProteinSW_HIDDEN_MATRIX(mat,i,j,INSERT) = NEGI;    
          ProteinSW_HIDDEN_MATRIX(mat,i,j,DELETE) = NEGI;    
          continue;  
          } /* end of Is not in envelope */ 


        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,MATCH) + 0;  
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,INSERT) + 0;  
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_HIDDEN_MATRIX(mat,i-1,j-1,DELETE) + 0;  
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
         score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));     
         ProteinSW_HIDDEN_MATRIX(mat,i,j,MATCH) = score; 
        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_HIDDEN_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;   
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_HIDDEN_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;   
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_HIDDEN_MATRIX(mat,i,j,INSERT) = score;    
        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_HIDDEN_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;   
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_HIDDEN_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;   
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_HIDDEN_MATRIX(mat,i,j,DELETE) = score;    
        /* Finished calculating state DELETE */ 
        }  
      }  


    return;  
}    


/* Function:  init_hidden_ProteinSW(mat,starti,startj,stopi,stopj)
 *
 * Descrip: No Description
 *
 * Arg:           mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:        starti [UNKN ] Undocumented argument [int]
 * Arg:        startj [UNKN ] Undocumented argument [int]
 * Arg:         stopi [UNKN ] Undocumented argument [int]
 * Arg:         stopj [UNKN ] Undocumented argument [int]
 *
 */
void init_hidden_ProteinSW(ProteinSW * mat,int starti,int startj,int stopi,int stopj) 
{
    register int i;  
    register int j;  
    register int hiddenj;    


    hiddenj = startj;    
    for(j=(startj-1);j<=stopj;j++)   {  
      for(i=(starti-1);i<=stopi;i++) {  
        ProteinSW_HIDDEN_MATRIX(mat,i,j,MATCH) = NEGI;
  
        ProteinSW_HIDDEN_MATRIX(mat,i,j,INSERT) = NEGI;
 
        ProteinSW_HIDDEN_MATRIX(mat,i,j,DELETE) = NEGI;
 
        }  
      }  


    return;  
}    


/* Function:  full_dc_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,out,donej,totalj,dpenv)
 *
 * Descrip:    The main divide-and-conquor routine. Basically, call /PackAln_calculate_small_ProteinSW
 *             Not this function, which is pretty hard core. 
 *             Function is given start/end points (in main matrix) for alignment
 *             It does some checks, decides whether start/end in j is small enough for explicit calc
 *               - if yes, calculates it, reads off into PackAln (out), adds the j distance to donej and returns TRUE
 *               - if no,  uses /do_dc_single_pass_ProteinSW to get mid-point
 *                          saves midpoint, and calls itself to do right portion then left portion
 *             right then left ensures PackAln is added the 'right' way, ie, back-to-front
 *             returns FALSE on any error, with a warning
 *
 *
 * Arg:               mat [UNKN ] Matrix with small memory implementation [ProteinSW *]
 * Arg:            starti [UNKN ] Start position in i [int]
 * Arg:            startj [UNKN ] Start position in j [int]
 * Arg:        startstate [UNKN ] Start position state number [int]
 * Arg:             stopi [UNKN ] Stop position in i [int]
 * Arg:             stopj [UNKN ] Stop position in j [int]
 * Arg:         stopstate [UNKN ] Stop position state number [int]
 * Arg:               out [UNKN ] PackAln structure to put alignment into [PackAln *]
 * Arg:             donej [UNKN ] pointer to a number with the amount of alignment done [int *]
 * Arg:            totalj [UNKN ] total amount of alignment to do (in j coordinates) [int]
 * Arg:             dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean full_dc_ProteinSW(ProteinSW * mat,int starti,int startj,int startstate,int stopi,int stopj,int stopstate,PackAln * out,int * donej,int totalj,DPEnvelope * dpenv) 
{
    int lstarti; 
    int lstartj; 
    int lstate;  


    if( mat->basematrix->type != BASEMATRIX_TYPE_SHADOW) {  
      warn("*Very* bad error! - non shadow matrix type in full_dc_ProteinSW");   
      return FALSE;  
      }  


    if( starti == -1 || startj == -1 || startstate == -1 || stopi == -1 || stopstate == -1)  {  
      warn("In full dc program, passed bad indices, indices passed were %d:%d[%d] to %d:%d[%d]\n",starti,startj,startstate,stopi,stopj,stopstate);   
      return FALSE;  
      }  


    if( stopj - startj < 5)  {  
      log_full_error(REPORT,0,"[%d,%d][%d,%d] Explicit read off",starti,startj,stopi,stopj);/* Build hidden explicit matrix */ 
      calculate_hidden_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,dpenv);  
      *donej += (stopj - startj);   /* Now read it off into out */ 
      if( read_hidden_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,out) == FALSE)    {  
        warn("In full dc, at %d:%d,%d:%d got a bad hidden explicit read off... ",starti,startj,stopi,stopj); 
        return FALSE;    
        }  
      return TRUE;   
      }  


/* In actual divide and conquor */ 
    if( do_dc_single_pass_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,dpenv,(int)(*donej*100)/totalj) == FALSE) {  
      warn("In divide and conquor for ProteinSW, at bound %d:%d to %d:%d, unable to calculate midpoint. Problem!",starti,startj,stopi,stopj);    
      return FALSE;  
      }  


/* Ok... now we have to call on each side of the matrix */ 
/* We have to retrieve left hand side positions, as they will be vapped by the time we call LHS */ 
    lstarti= ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,0);     
    lstartj= ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,1);     
    lstate = ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,2);     


/* Call on right hand side: this lets us do the correct read off */ 
    if( full_dc_ProteinSW(mat,ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,3),ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,4),ProteinSW_DC_SHADOW_MATRIX_SP(mat,stopi,stopj,stopstate,5),stopi,stopj,stopstate,out,donej,totalj,dpenv) == FALSE)   {  
/* Warning already issued, simply chained back up to top */ 
      return FALSE;  
      }  
/* Call on left hand side */ 
    if( full_dc_ProteinSW(mat,starti,startj,startstate,lstarti,lstartj,lstate,out,donej,totalj,dpenv) == FALSE)  {  
/* Warning already issued, simply chained back up to top */ 
      return FALSE;  
      }  


    return TRUE;     
}    


/* Function:  do_dc_single_pass_ProteinSW(mat,starti,startj,startstate,stopi,stopj,stopstate,dpenv,perc_done)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:            starti [UNKN ] Undocumented argument [int]
 * Arg:            startj [UNKN ] Undocumented argument [int]
 * Arg:        startstate [UNKN ] Undocumented argument [int]
 * Arg:             stopi [UNKN ] Undocumented argument [int]
 * Arg:             stopj [UNKN ] Undocumented argument [int]
 * Arg:         stopstate [UNKN ] Undocumented argument [int]
 * Arg:             dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 * Arg:         perc_done [UNKN ] Undocumented argument [int]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean do_dc_single_pass_ProteinSW(ProteinSW * mat,int starti,int startj,int startstate,int stopi,int stopj,int stopstate,DPEnvelope * dpenv,int perc_done) 
{
    int halfj;   
    halfj = startj + ((stopj - startj)/2);   


    init_dc_ProteinSW(mat);  


    ProteinSW_DC_SHADOW_MATRIX(mat,starti,startj,startstate) = 0;    
    run_up_dc_ProteinSW(mat,starti,stopi,startj,halfj-1,dpenv,perc_done);    
    push_dc_at_merge_ProteinSW(mat,starti,stopi,halfj,&halfj,dpenv);     
    follow_on_dc_ProteinSW(mat,starti,stopi,halfj,stopj,dpenv,perc_done);    
    return TRUE; 
}    


/* Function:  push_dc_at_merge_ProteinSW(mat,starti,stopi,startj,stopj,dpenv)
 *
 * Descrip: No Description
 *
 * Arg:           mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:        starti [UNKN ] Undocumented argument [int]
 * Arg:         stopi [UNKN ] Undocumented argument [int]
 * Arg:        startj [UNKN ] Undocumented argument [int]
 * Arg:         stopj [UNKN ] Undocumented argument [int *]
 * Arg:         dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 */
void push_dc_at_merge_ProteinSW(ProteinSW * mat,int starti,int stopi,int startj,int * stopj,DPEnvelope * dpenv) 
{
    register int i;  
    register int j;  
    register int k;  
    register int count;  
    register int mergej;/* Sources below this j will be stamped by triples */ 
    register int score;  
    register int temp;   


    mergej = startj -1;  
    for(count=0,j=startj;count<1;count++,j++)    {  
      for(i=starti;i<=stopi;i++) {  
        if( dpenv != NULL && is_in_DPEnvelope(dpenv,i,j) == FALSE )  {  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,0) = (-100);   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,1) = (-100);   
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;     
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,0) = (-100);  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,1) = (-100);  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;     
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,0) = (-100);  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,1) = (-100);  
          continue;  
          } /* end of Is not in envelope */ 


        /* For state MATCH, pushing when j - offj <= mergej */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,MATCH) + 0;   
        if( j - 1 <= mergej) {  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,0) = i-1;  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,1) = j-1;  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,2) = MATCH;    
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,3) = i;    
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,4) = j;    
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,5) = MATCH;    
          }  
        else {  
          for(k=0;k<7;k++)   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,MATCH,k); 
          }  


        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,INSERT) + 0;   
        if( temp > score)    {  
          score = temp;  


          if( j - 1 <= mergej)   {  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,0) = i-1;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,1) = j-1;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,2) = INSERT; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,3) = i;  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,4) = j;  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,5) = MATCH;  
            }  
          else   {  
            for(k=0;k<7;k++) 
              ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,INSERT,k);  
            }  
          }  


        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,DELETE) + 0;   
        if( temp > score)    {  
          score = temp;  


          if( j - 1 <= mergej)   {  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,0) = i-1;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,1) = j-1;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,2) = DELETE; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,3) = i;  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,4) = j;  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,5) = MATCH;  
            }  
          else   {  
            for(k=0;k<7;k++) 
              ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,DELETE,k);  
            }  
          }  
        /* Add any movement independant score */ 
        score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));  
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = score;   
        /* Finished with state MATCH */ 


        /* For state INSERT, pushing when j - offj <= mergej */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;    
        if( j - 1 <= mergej) {  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,0) = i-0; 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,1) = j-1; 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,2) = MATCH;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,3) = i;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,4) = j;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,5) = INSERT;  
          }  
        else {  
          for(k=0;k<7;k++)   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,MATCH,k);    
          }  


        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;    
        if( temp > score)    {  
          score = temp;  


          if( j - 1 <= mergej)   {  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,0) = i-0;   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,1) = j-1;   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,2) = INSERT;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,3) = i; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,4) = j; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,5) = INSERT;    
            }  
          else   {  
            for(k=0;k<7;k++) 
              ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,INSERT,k); 
            }  
          }  
        /* Add any movement independant score */ 
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = score;  
        /* Finished with state INSERT */ 


        /* For state DELETE, pushing when j - offj <= mergej */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;    
        if( j - 0 <= mergej) {  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,0) = i-1; 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,1) = j-0; 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,2) = MATCH;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,3) = i;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,4) = j;   
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,5) = DELETE;  
          }  
        else {  
          for(k=0;k<7;k++)   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,MATCH,k);    
          }  


        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;    
        if( temp > score)    {  
          score = temp;  


          if( j - 0 <= mergej)   {  
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,0) = i-1;   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,1) = j-0;   
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,2) = DELETE;    
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,3) = i; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,4) = j; 
            ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,5) = DELETE;    
            }  
          else   {  
            for(k=0;k<7;k++) 
              ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,DELETE,k); 
            }  
          }  
        /* Add any movement independant score */ 
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = score;  
        /* Finished with state DELETE */ 
        }  
      }  
    /* Put back j into * stop j so that calling function gets it correct */ 
    if( stopj == NULL)   
      warn("Bad news... NULL stopj pointer in push dc function. This means that calling function does not know how many cells I have done!");    
    else 
      *stopj = j;    


    return;  
}    


/* Function:  follow_on_dc_ProteinSW(mat,starti,stopi,startj,stopj,dpenv,perc_done)
 *
 * Descrip: No Description
 *
 * Arg:              mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:           starti [UNKN ] Undocumented argument [int]
 * Arg:            stopi [UNKN ] Undocumented argument [int]
 * Arg:           startj [UNKN ] Undocumented argument [int]
 * Arg:            stopj [UNKN ] Undocumented argument [int]
 * Arg:            dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 * Arg:        perc_done [UNKN ] Undocumented argument [int]
 *
 */
void follow_on_dc_ProteinSW(ProteinSW * mat,int starti,int stopi,int startj,int stopj,DPEnvelope * dpenv,int perc_done) 
{
    int i;   
    int j;   
    int k;   
    int score;   
    int temp;    
    int localshadow[7];  
    long int total;  
    long int num;    


    total = (stopi - starti+1) * (stopj - startj+1); 
    num = 0;     


    for(j=startj;j<=stopj;j++)   {  
      for(i=starti;i<=stopi;i++) {  
        num++;   
        if( dpenv != NULL && is_in_DPEnvelope(dpenv,i,j) == FALSE )  {  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;     
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;     
          continue;  
          } /* end of Is not in envelope */ 
        if( num % 1000 == 0 )    
          log_full_error(REPORT,0,"[%d%%%% done]After  mid-j %5d Cells done %d%%%%",perc_done,startj,(num*100)/total);   


        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,MATCH) + 0;   
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,MATCH,k);   
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,INSERT) + 0;   
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,INSERT,k);    
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,DELETE) + 0;   
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,DELETE,k);    
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
         score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));     
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = score;  
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = localshadow[k];   
        /* Now figure out if any specials need this score */ 
        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;    
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,MATCH,k);   
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;    
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,INSERT,k);    
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = score; 
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,k) = localshadow[k];  
        /* Now figure out if any specials need this score */ 
        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;    
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,MATCH,k);   
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;    
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,DELETE,k);    
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = score; 
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,k) = localshadow[k];  
        /* Now figure out if any specials need this score */ 
        /* Finished calculating state DELETE */ 
        } /* end of this is strip */ 
      } /* end of for each valid j column */ 


/* Function:  run_up_dc_ProteinSW(mat,starti,stopi,startj,stopj,dpenv,perc_done)
 *
 * Descrip: No Description
 *
 * Arg:              mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:           starti [UNKN ] Undocumented argument [int]
 * Arg:            stopi [UNKN ] Undocumented argument [int]
 * Arg:           startj [UNKN ] Undocumented argument [int]
 * Arg:            stopj [UNKN ] Undocumented argument [int]
 * Arg:            dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 * Arg:        perc_done [UNKN ] Undocumented argument [int]
 *
 */
}    
void run_up_dc_ProteinSW(ProteinSW * mat,int starti,int stopi,int startj,int stopj,DPEnvelope * dpenv,int perc_done) 
{
    register int i;  
    register int j;  
    register int score;  
    register int temp;   
    long int total;  
    long int num;    


    total = (stopi - starti+1) * (stopj - startj+1); 
    if( total <= 0 ) 
      total = 1; 
    num = 0;     


    for(j=startj;j<=stopj;j++)   {  
      for(i=starti;i<=stopi;i++) {  
        if( j == startj && i == starti)  
          continue;  
        num++;   
        if( dpenv != NULL && is_in_DPEnvelope(dpenv,i,j) == FALSE )  {  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;     
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;     
          continue;  
          } /* end of Is not in envelope */ 
        if( num % 1000 == 0 )    
          log_full_error(REPORT,0,"[%d%%%% done]Before mid-j %5d Cells done %d%%%%",perc_done,stopj,(num*100)/total);    


        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,MATCH) + 0;   
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,INSERT) + 0;   
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,DELETE) + 0;   
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
         score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));     
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = score;  
        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;    
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;    
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = score; 
        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;    
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;    
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = score; 
        /* Finished calculating state DELETE */ 
        } /* end of this is strip */ 
      } /* end of for each valid j column */ 


/* Function:  init_dc_ProteinSW(mat)
 *
 * Descrip: No Description
 *
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 */
}    
void init_dc_ProteinSW(ProteinSW * mat) 
{
    register int i;  
    register int j;  
    register int k;  


    for(j=0;j<3;j++) {  
      for(i=(-1);i<mat->query->seq->len;i++) {  
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;    
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;   
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;   
        for(k=0;k<7;k++) {  
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = (-1); 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,k) = (-1);    
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,k) = (-1);    
          }  
        }  
      }  


    return;  
}    


/* Function:  dc_start_end_calculate_ProteinSW(mat,dpenv)
 *
 * Descrip:    Calculates special strip, leaving start/end/score points in the shadow matrix 
 *             One tricky thing is that we need to add score-independent calcs in the states
 *             As we have to evaluate them then. This is not ideally implemented therefore 
 *             In fact it is *definitely* not ideal. Will have to do for now
 *
 *
 * Arg:          mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:        dpenv [UNKN ] Undocumented argument [DPEnvelope *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean dc_start_end_calculate_ProteinSW(ProteinSW * mat,DPEnvelope * dpenv) 
{
    int i;   
    int j;   
    int k;   
    int score;   
    int temp;    
    int leni;    
    int lenj;    
    int localshadow[7];  
    long int total;  
    long int num=0;  


    init_start_end_linear_ProteinSW(mat);    


    leni = mat->query->seq->len; 
    lenj = mat->target->seq->len;    
    total = leni * lenj; 


    for(j=0;j<lenj;j++)  {  
      for(i=0;i<leni;i++)    {  
        num++;   
        if( dpenv != NULL && is_in_DPEnvelope(dpenv,i,j) == FALSE )  {  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;  
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;     
          ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;     
          continue;  
          } /* end of Is not in envelope */ 
        if( num%1000 == 0)   
          log_full_error(REPORT,0,"%6d Cells done [%2d%%%%]",num,num*100/total); 




        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,MATCH) + 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));     
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,MATCH,k);   
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,INSERT) + 0 +(CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,INSERT,k);    
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-1,DELETE) + 0 +(CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 1,DELETE,k);    
          }  
        /* From state START to state MATCH */ 
        temp = ProteinSW_DC_SHADOW_SPECIAL(mat,i-1,j-1,START) + 0 + (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));     
        if( temp  > score )  {  
          score = temp;  
          /* This state [START] is a special for MATCH... push top shadow pointers here */ 
          localshadow[0]= i; 
          localshadow[1]= j; 
          localshadow[2]= MATCH; 
          localshadow[3]= (-1);  
          localshadow[4]= (-1);  
          localshadow[5]= (-1);  
          localshadow[6]= score; 
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
        /* Actually, already done inside scores */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = score;  
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k) = localshadow[k];   
        /* Now figure out if any specials need this score */ 


        /* state MATCH is a source for special END */ 
        temp = score + (0) + (0) ;   
        if( temp > ProteinSW_DC_SHADOW_SPECIAL(mat,i,j,END) )    {  
          ProteinSW_DC_SHADOW_SPECIAL(mat,i,j,END) = temp;   
          /* Have to push only bottem half of system here */ 
          for(k=0;k<3;k++)   
            ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,END,k) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,k);  
          ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,END,6) = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,6);    
          ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,END,3) = i; 
          ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,END,4) = j; 
          ProteinSW_DC_SHADOW_SPECIAL_SP(mat,i,j,END,5) = MATCH; 
          }  




        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,MATCH) + mat->gap + (0);  
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,MATCH,k);   
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-0,j-1,INSERT) + mat->ext +(0);   
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 0,j - 1,INSERT,k);    
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
        /* Actually, already done inside scores */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = score; 
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,k) = localshadow[k];  
        /* Now figure out if any specials need this score */ 


        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,MATCH) + mat->gap + (0);  
        /* shift first shadow numbers */ 
        for(k=0;k<7;k++) 
          localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,MATCH,k);   
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_DC_SHADOW_MATRIX(mat,i-1,j-0,DELETE) + mat->ext +(0);   
        if( temp  > score )  {  
          score = temp;  
          for(k=0;k<7;k++)   
            localshadow[k] = ProteinSW_DC_SHADOW_MATRIX_SP(mat,i - 1,j - 0,DELETE,k);    
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
        /* Actually, already done inside scores */ 
         ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = score; 
        for(k=0;k<7;k++) 
          ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,k) = localshadow[k];  
        /* Now figure out if any specials need this score */ 


        /* Finished calculating state DELETE */ 


        } /* end of for each i position in strip */ 
      } /* end of for each j strip */ 
    return TRUE;     
}    


/* Function:  start_end_find_end_ProteinSW(mat,endj)
 *
 * Descrip:    First function used to find end of the best path in the special state !end
 *
 *
 * Arg:         mat [UNKN ] Matrix in small mode [ProteinSW *]
 * Arg:        endj [WRITE] position of end in j (meaningless in i) [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int start_end_find_end_ProteinSW(ProteinSW * mat,int * endj) 
{
    register int j;  
    register int max;    
    register int maxj;   


    max = ProteinSW_DC_SHADOW_SPECIAL(mat,0,mat->target->seq->len-1,END);    
    maxj = mat->target->seq->len-1;  
    for(j= mat->target->seq->len-2 ;j >= 0 ;j--) {  
      if( ProteinSW_DC_SHADOW_SPECIAL(mat,0,j,END) > max )   {  
        max = ProteinSW_DC_SHADOW_SPECIAL(mat,0,j,END);  
        maxj = j;    
        }  
      }  


    if( endj != NULL)    
      *endj = maxj;  


    return max;  
}    


/* Function:  init_start_end_linear_ProteinSW(mat)
 *
 * Descrip: No Description
 *
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 */
void init_start_end_linear_ProteinSW(ProteinSW * mat) 
{
    register int i;  
    register int j;  
    for(j=0;j<3;j++) {  
      for(i=(-1);i<mat->query->seq->len;i++) {  
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,MATCH) = NEGI;    
        ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,MATCH,0) = (-1);   
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,INSERT) = NEGI;   
        ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,INSERT,0) = (-1);  
        ProteinSW_DC_SHADOW_MATRIX(mat,i,j,DELETE) = NEGI;   
        ProteinSW_DC_SHADOW_MATRIX_SP(mat,i,j,DELETE,0) = (-1);  
        }  
      }  


    for(j=(-1);j<mat->target->seq->len;j++)  {  
      ProteinSW_DC_SHADOW_SPECIAL(mat,0,j,START) = 0;    
      ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,j,START,0) = j;   
      ProteinSW_DC_SHADOW_SPECIAL(mat,0,j,END) = NEGI;   
      ProteinSW_DC_SHADOW_SPECIAL_SP(mat,0,j,END,0) = (-1);  
      }  


    return;  
}    


/* Function:  AlnRangeSet_from_PackAln_ProteinSW(pal)
 *
 * Descrip:    Converts a PackAln (full alignment) to start/stop range
 *             The point being that you may have the PackAln and someone wants a range
 *
 *
 * Arg:        pal [UNKN ] Undocumented argument [PackAln *]
 *
 * Return [UNKN ]  Undocumented return value [AlnRangeSet *]
 *
 */
AlnRangeSet * AlnRangeSet_from_PackAln_ProteinSW(PackAln * pal) 
{
    int unit;    
    int cum_score = 0;   
    AlnRangeSet * out;   
    AlnRange * temp; 


    out = AlnRangeSet_alloc_std();   
    for(unit = 0;unit < pal->len;unit++) {  
      cum_score += pal->pau[unit]->score;    
      if( pal->pau[unit]->state < 3) {  
        /* It is out of the specials */ 
        temp = AlnRange_alloc(); 
        temp->starti = pal->pau[unit]->i;    
        temp->startj = pal->pau[unit]->j;    
        temp->startstate = pal->pau[unit]->state;    
        temp->startscore = cum_score;    
        for(;unit < pal->len && pal->pau[unit]->state < 3;unit++)    
          cum_score += pal->pau[unit]->score;    
        temp->stopi = pal->pau[unit-1]->i;   
        temp->stopj = pal->pau[unit-1]->j;   
        temp->stopstate = pal->pau[unit-1]->state;   
        temp->stopscore = cum_score; 
        add_AlnRangeSet(out,temp);   
        }  
      } /* end of while there are more units */ 


    return out;  
}    


/* Function:  convert_PackAln_to_AlnBlock_ProteinSW(pal)
 *
 * Descrip:    Converts a path alignment to a label alignment
 *             The label alignment is probably much more useful than the path
 *
 *
 * Arg:        pal [UNKN ] Undocumented argument [PackAln *]
 *
 * Return [UNKN ]  Undocumented return value [AlnBlock *]
 *
 */
AlnBlock * convert_PackAln_to_AlnBlock_ProteinSW(PackAln * pal) 
{
    AlnConvertSet * acs; 
    AlnBlock * alb;  


    acs = AlnConvertSet_ProteinSW(); 
    alb = AlnBlock_from_PackAln(acs,pal);    
    free_AlnConvertSet(acs); 
    return alb;  
}    


 static char * query_label[] = { "SEQUENCE","INSERT","END" };    
/* Function:  AlnConvertSet_ProteinSW(void)
 *
 * Descrip: No Description
 *
 *
 * Return [UNKN ]  Undocumented return value [AlnConvertSet *]
 *
 */
 static char * target_label[] = { "SEQUENCE","INSERT","END" };   
AlnConvertSet * AlnConvertSet_ProteinSW(void) 
{
    AlnConvertUnit * acu;    
    AlnConvertSet  * out;    


    out = AlnConvertSet_alloc_std(); 


    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = MATCH; 
    acu->state2 = MATCH;     
    acu->offi = 1;   
    acu->offj = 1;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = INSERT;    
    acu->state2 = MATCH;     
    acu->offi = 1;   
    acu->offj = 1;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = DELETE;    
    acu->state2 = MATCH;     
    acu->offi = 1;   
    acu->offj = 1;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = START + 3; 
    acu->is_from_special = TRUE; 
    acu->state2 = MATCH;     
    acu->offi = (-1);    
    acu->offj = 1;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = MATCH; 
    acu->state2 = INSERT;    
    acu->offi = 0;   
    acu->offj = 1;   
    acu->label1 = query_label[1];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = INSERT;    
    acu->state2 = INSERT;    
    acu->offi = 0;   
    acu->offj = 1;   
    acu->label1 = query_label[1];    
    acu->label2 = target_label[0];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = MATCH; 
    acu->state2 = DELETE;    
    acu->offi = 1;   
    acu->offj = 0;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[1];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = DELETE;    
    acu->state2 = DELETE;    
    acu->offi = 1;   
    acu->offj = 0;   
    acu->label1 = query_label[0];    
    acu->label2 = target_label[1];   
    acu = AlnConvertUnit_alloc();    
    add_AlnConvertSet(out,acu);  
    acu->state1 = MATCH; 
    acu->state2 = END + 3;   
    acu->offi = (-1);    
    acu->offj = 0;   
    acu->label1 = query_label[2];    
    acu->label2 = target_label[2];   
    return out;  
}    


/* Function:  PackAln_read_Expl_ProteinSW(mat)
 *
 * Descrip:    Reads off PackAln from explicit matrix structure
 *
 *
 * Arg:        mat [UNKN ] Undocumented argument [ProteinSW *]
 *
 * Return [UNKN ]  Undocumented return value [PackAln *]
 *
 */
PackAln * PackAln_read_Expl_ProteinSW(ProteinSW * mat) 
{
    register PackAln * out;  
    int i;   
    int j;   
    int state;   
    int cellscore = (-1);    
    boolean isspecial;   
    PackAlnUnit * pau = NULL;    
    PackAlnUnit * prev = NULL;   


    if( mat->basematrix->type != BASEMATRIX_TYPE_EXPLICIT)   {  
      warn("In ProteinSW_basic_read You have asked for an alignment from a non-explicit matrix: c'est impossible [current type is %d - %s]", mat->basematrix->type,basematrix_type_to_string(mat->basematrix->type));    
      return NULL;   
      }  


    out = PackAln_alloc_std();   
    if( out == NULL )    
      return NULL;   


    out->score =  find_end_ProteinSW(mat,&i,&j,&state,&isspecial);   


    /* Add final end transition (at the moment we have not got the score! */ 
    if( (pau= PackAlnUnit_alloc()) == NULL  || add_PackAln(out,pau) == FALSE )   {  
      warn("Failed the first PackAlnUnit alloc, %d length of Alignment in ProteinSW_basic_read, returning a mess.(Sorry!)",out->len);    
      return out;    
      }  


    /* Put in positions for end trans. Remember that coordinates in C style */ 
    pau->i = i;  
    pau->j = j;  
    if( isspecial != TRUE)   
      pau->state = state;    
    else pau->state = state + 3;     
    prev=pau;    
    while( state != START || isspecial != TRUE)  {  


      if( isspecial == TRUE )    
        max_calc_special_ProteinSW(mat,i,j,state,isspecial,&i,&j,&state,&isspecial,&cellscore);  
      else   
        max_calc_ProteinSW(mat,i,j,state,isspecial,&i,&j,&state,&isspecial,&cellscore);  
      if(i == ProteinSW_READ_OFF_ERROR || j == ProteinSW_READ_OFF_ERROR || state == ProteinSW_READ_OFF_ERROR )   {  
        warn("Problem - hit bad read off system, exiting now");  
        break;   
        }  
      if( (pau= PackAlnUnit_alloc()) == NULL  || add_PackAln(out,pau) == FALSE ) {  
        warn("Failed a PackAlnUnit alloc, %d length of Alignment in ProteinSW_basic_read, returning partial alignment",out->len);    
        break;   
        }  


      /* Put in positions for block. Remember that coordinates in C style */ 
      pau->i = i;    
      pau->j = j;    
      if( isspecial != TRUE)     
        pau->state = state;  
      else pau->state = state + 3;   
      prev->score = cellscore;   
      prev = pau;    
      } /* end of while state != START */ 


    invert_PackAln(out); 
    return out;  
}    


/* Function:  find_end_ProteinSW(mat,ri,rj,state,isspecial)
 *
 * Descrip: No Description
 *
 * Arg:              mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:               ri [UNKN ] Undocumented argument [int *]
 * Arg:               rj [UNKN ] Undocumented argument [int *]
 * Arg:            state [UNKN ] Undocumented argument [int *]
 * Arg:        isspecial [UNKN ] Undocumented argument [boolean *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int find_end_ProteinSW(ProteinSW * mat,int * ri,int * rj,int * state,boolean * isspecial) 
{
    register int j;  
    register int max;    
    register int maxj;   


    max = ProteinSW_EXPL_SPECIAL(mat,0,mat->target->seq->len-1,END); 
    maxj = mat->target->seq->len-1;  
    for(j= mat->target->seq->len-2 ;j >= 0 ;j--) {  
      if( ProteinSW_EXPL_SPECIAL(mat,0,j,END) > max )    {  
        max = ProteinSW_EXPL_SPECIAL(mat,0,j,END);   
        maxj = j;    
        }  
      }  


    if( ri != NULL)  
       *ri = 0;  
    if( rj != NULL)  
       *rj = maxj;   
    if( state != NULL)   
       *state = END; 
    if( isspecial != NULL)   
       *isspecial = TRUE;    


    return max;  
}    


/* Function:  ProteinSW_debug_show_matrix(mat,starti,stopi,startj,stopj,ofp)
 *
 * Descrip: No Description
 *
 * Arg:           mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:        starti [UNKN ] Undocumented argument [int]
 * Arg:         stopi [UNKN ] Undocumented argument [int]
 * Arg:        startj [UNKN ] Undocumented argument [int]
 * Arg:         stopj [UNKN ] Undocumented argument [int]
 * Arg:           ofp [UNKN ] Undocumented argument [FILE *]
 *
 */
void ProteinSW_debug_show_matrix(ProteinSW * mat,int starti,int stopi,int startj,int stopj,FILE * ofp) 
{
    register int i;  
    register int j;  


    for(i=starti;i<stopi && i < mat->query->seq->len;i++)    {  
      for(j=startj;j<stopj && j < mat->target->seq->len;j++) {  
        fprintf(ofp,"Cell [%d - %d]\n",i,j);     
        fprintf(ofp,"State MATCH %d\n",ProteinSW_EXPL_MATRIX(mat,i,j,MATCH));    
        fprintf(ofp,"State INSERT %d\n",ProteinSW_EXPL_MATRIX(mat,i,j,INSERT));  
        fprintf(ofp,"State DELETE %d\n",ProteinSW_EXPL_MATRIX(mat,i,j,DELETE));  
        fprintf(ofp,"\n\n"); 
        }  
      }  


}    


/* Function:  max_calc_ProteinSW(mat,i,j,state,isspecial,reti,retj,retstate,retspecial,cellscore)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:                 i [UNKN ] Undocumented argument [int]
 * Arg:                 j [UNKN ] Undocumented argument [int]
 * Arg:             state [UNKN ] Undocumented argument [int]
 * Arg:         isspecial [UNKN ] Undocumented argument [boolean]
 * Arg:              reti [UNKN ] Undocumented argument [int *]
 * Arg:              retj [UNKN ] Undocumented argument [int *]
 * Arg:          retstate [UNKN ] Undocumented argument [int *]
 * Arg:        retspecial [UNKN ] Undocumented argument [boolean *]
 * Arg:         cellscore [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int max_calc_ProteinSW(ProteinSW * mat,int i,int j,int state,boolean isspecial,int * reti,int * retj,int * retstate,boolean * retspecial,int * cellscore) 
{
    register int temp;   
    register int cscore; 


    *reti = (*retj) = (*retstate) = ProteinSW_READ_OFF_ERROR;    


    if( i < 0 || j < 0 || i > mat->query->seq->len || j > mat->target->seq->len) {  
      warn("In ProteinSW matrix special read off - out of bounds on matrix [i,j is %d,%d state %d in standard matrix]",i,j,state);   
      return -1;     
      }  


    /* Then you have to select the correct switch statement to figure out the readoff      */ 
    /* Somewhat odd - reverse the order of calculation and return as soon as it is correct */ 
    cscore = ProteinSW_EXPL_MATRIX(mat,i,j,state);   
    switch(state)    {  
      case MATCH :   
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_EXPL_SPECIAL(mat,i - 1,j - 1,START) )  {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = START; 
          *retspecial = TRUE;    
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_SPECIAL(mat,i-1,j-1,START); 
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,START);   
          }  
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,DELETE) )  {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = DELETE;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-1,j-1,DELETE); 
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,DELETE);  
          }  
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,INSERT) )  {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = INSERT;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-1,j-1,INSERT); 
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,INSERT);  
          }  
        temp = cscore - (0) -  (CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j)));  
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,MATCH) )   {  
          *reti = i - 1; 
          *retj = j - 1; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-1,j-1,MATCH);  
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 1,MATCH);   
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      case INSERT :  
        temp = cscore - (mat->ext) -  (0);   
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 0,j - 1,INSERT) )  {  
          *reti = i - 0; 
          *retj = j - 1; 
          *retstate = INSERT;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-0,j-1,INSERT); 
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 0,j - 1,INSERT);  
          }  
        temp = cscore - (mat->gap) -  (0);   
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 0,j - 1,MATCH) )   {  
          *reti = i - 0; 
          *retj = j - 1; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-0,j-1,MATCH);  
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 0,j - 1,MATCH);   
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      case DELETE :  
        temp = cscore - (mat->ext) -  (0);   
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 1,j - 0,DELETE) )  {  
          *reti = i - 1; 
          *retj = j - 0; 
          *retstate = DELETE;    
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-1,j-0,DELETE); 
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 0,DELETE);  
          }  
        temp = cscore - (mat->gap) -  (0);   
        if( temp == ProteinSW_EXPL_MATRIX(mat,i - 1,j - 0,MATCH) )   {  
          *reti = i - 1; 
          *retj = j - 0; 
          *retstate = MATCH; 
          *retspecial = FALSE;   
          if( cellscore != NULL) {  
            *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-1,j-0,MATCH);  
            }  
          return ProteinSW_EXPL_MATRIX(mat,i - 1,j - 0,MATCH);   
          }  
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      default:   
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found!",i,j,state);   
        return (-1); 
      } /* end of Switch state  */ 
}    


/* Function:  max_calc_special_ProteinSW(mat,i,j,state,isspecial,reti,retj,retstate,retspecial,cellscore)
 *
 * Descrip: No Description
 *
 * Arg:               mat [UNKN ] Undocumented argument [ProteinSW *]
 * Arg:                 i [UNKN ] Undocumented argument [int]
 * Arg:                 j [UNKN ] Undocumented argument [int]
 * Arg:             state [UNKN ] Undocumented argument [int]
 * Arg:         isspecial [UNKN ] Undocumented argument [boolean]
 * Arg:              reti [UNKN ] Undocumented argument [int *]
 * Arg:              retj [UNKN ] Undocumented argument [int *]
 * Arg:          retstate [UNKN ] Undocumented argument [int *]
 * Arg:        retspecial [UNKN ] Undocumented argument [boolean *]
 * Arg:         cellscore [UNKN ] Undocumented argument [int *]
 *
 * Return [UNKN ]  Undocumented return value [int]
 *
 */
int max_calc_special_ProteinSW(ProteinSW * mat,int i,int j,int state,boolean isspecial,int * reti,int * retj,int * retstate,boolean * retspecial,int * cellscore) 
{
    register int temp;   
    register int cscore; 


    *reti = (*retj) = (*retstate) = ProteinSW_READ_OFF_ERROR;    


    if( j < 0 || j > mat->target->seq->len)  {  
      warn("In ProteinSW matrix special read off - out of bounds on matrix [j is %d in special]",j); 
      return -1;     
      }  


    cscore = ProteinSW_EXPL_SPECIAL(mat,i,j,state);  
    switch(state)    {  
      case START :   
      case END :     
        /* source MATCH is from main matrix */ 
        for(i= mat->query->seq->len-1;i >= 0 ;i--)   {  
          temp = cscore - (0) - (0);     
          if( temp == ProteinSW_EXPL_MATRIX(mat,i - 0,j - 0,MATCH) ) {  
            *reti = i - 0;   
            *retj = j - 0;   
            *retstate = MATCH;   
            *retspecial = FALSE; 
            if( cellscore != NULL)   {  
              *cellscore = cscore - ProteinSW_EXPL_MATRIX(mat,i-0,j-0,MATCH);    
              }  
            return ProteinSW_EXPL_MATRIX(mat,i - 0,j - 0,MATCH) ;    
            }  
          } /* end of for i >= 0 */ 
      default:   
        warn("Major problem (!) - in ProteinSW read off, position %d,%d state %d no source found  dropped into default on source switch!",i,j,state);    
        return (-1); 
      } /* end of switch on special states */ 
}    


/* Function:  calculate_ProteinSW(mat)
 *
 * Descrip:    This function calculates the ProteinSW matrix when in explicit mode
 *             To allocate the matrix use /allocate_Expl_ProteinSW
 *
 *
 * Arg:        mat [UNKN ] ProteinSW which contains explicit basematrix memory [ProteinSW *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
boolean calculate_ProteinSW(ProteinSW * mat) 
{
    int i;   
    int j;   
    int leni;    
    int lenj;    
    int tot; 
    int num; 


    if( mat->basematrix->type != BASEMATRIX_TYPE_EXPLICIT )  {  
      warn("in calculate_ProteinSW, passed a non Explicit matrix type, cannot calculate!");  
      return FALSE;  
      }  


    leni = mat->leni;    
    lenj = mat->lenj;    
    tot = leni * lenj;   
    num = 0; 


    start_reporting("ProteinSW Matrix calculation: ");   
    for(j=0;j<lenj;j++)  {  
      auto int score;    
      auto int temp;     
      for(i=0;i<leni;i++)    {  
        if( num%1000 == 0 )  
          log_full_error(REPORT,0,"[%7d] Cells %2d%%%%",num,num*100/tot);    
        num++;   


        /* For state MATCH */ 
        /* setting first movement to score */ 
        score = ProteinSW_EXPL_MATRIX(mat,i-1,j-1,MATCH) + 0;    
        /* From state INSERT to state MATCH */ 
        temp = ProteinSW_EXPL_MATRIX(mat,i-1,j-1,INSERT) + 0;    
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state DELETE to state MATCH */ 
        temp = ProteinSW_EXPL_MATRIX(mat,i-1,j-1,DELETE) + 0;    
        if( temp  > score )  {  
          score = temp;  
          }  
        /* From state START to state MATCH */ 
        temp = ProteinSW_EXPL_SPECIAL(mat,i-1,j-1,START) + 0;    
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for MATCH */ 
        /* Add any movement independant score and put away */ 
         score += CompMat_AAMATCH(mat->comp,CSEQ_PROTEIN_AMINOACID(mat->query,i),CSEQ_PROTEIN_AMINOACID(mat->target,j));     
         ProteinSW_EXPL_MATRIX(mat,i,j,MATCH) = score;   


        /* state MATCH is a source for special END */ 
        temp = score + (0) + (0) ;   
        if( temp > ProteinSW_EXPL_SPECIAL(mat,i,j,END) )     {  
          ProteinSW_EXPL_SPECIAL(mat,i,j,END) = temp;    
          }  




        /* Finished calculating state MATCH */ 


        /* For state INSERT */ 
        /* setting first movement to score */ 
        score = ProteinSW_EXPL_MATRIX(mat,i-0,j-1,MATCH) + mat->gap;     
        /* From state INSERT to state INSERT */ 
        temp = ProteinSW_EXPL_MATRIX(mat,i-0,j-1,INSERT) + mat->ext;     
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for INSERT */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_EXPL_MATRIX(mat,i,j,INSERT) = score;  


        /* Finished calculating state INSERT */ 


        /* For state DELETE */ 
        /* setting first movement to score */ 
        score = ProteinSW_EXPL_MATRIX(mat,i-1,j-0,MATCH) + mat->gap;     
        /* From state DELETE to state DELETE */ 
        temp = ProteinSW_EXPL_MATRIX(mat,i-1,j-0,DELETE) + mat->ext;     
        if( temp  > score )  {  
          score = temp;  
          }  


        /* Ok - finished max calculation for DELETE */ 
        /* Add any movement independant score and put away */ 
         ProteinSW_EXPL_MATRIX(mat,i,j,DELETE) = score;  


        /* Finished calculating state DELETE */ 
        }  


      /* Special state START has no special to special movements */ 


      /* Special state END has no special to special movements */ 
      }  
    stop_reporting();    
    return TRUE;     
}    


/* Function:  ProteinSW_alloc(void)
 *
 * Descrip:    Allocates structure: assigns defaults if given 
 *
 *
 *
 * Return [UNKN ]  Undocumented return value [ProteinSW *]
 *
 */
ProteinSW * ProteinSW_alloc(void) 
{
    ProteinSW * out;/* out is exported at end of function */ 


    /* call ckalloc and see if NULL */ 
    if((out=(ProteinSW *) ckalloc (sizeof(ProteinSW))) == NULL)  {  
      warn("ProteinSW_alloc failed ");   
      return NULL;  /* calling function should respond! */ 
      }  
    out->dynamite_hard_link = 1; 
    out->basematrix = NULL;  
    out->leni = 0;   
    out->lenj = 0;   


    return out;  
}    


/* Function:  free_ProteinSW(obj)
 *
 * Descrip:    Free Function: removes the memory held by obj
 *             Will chain up to owned members and clear all lists
 *
 *
 * Arg:        obj [UNKN ] Object that is free'd [ProteinSW *]
 *
 * Return [UNKN ]  Undocumented return value [ProteinSW *]
 *
 */
ProteinSW * free_ProteinSW(ProteinSW * obj) 
{


    if( obj == NULL) {  
      warn("Attempting to free a NULL pointer to a ProteinSW obj. Should be trappable"); 
      return NULL;   
      }  


    if( obj->dynamite_hard_link > 1)     {  
      obj->dynamite_hard_link--; 
      return NULL;   
      }  
    if( obj->basematrix != NULL) 
      free_BaseMatrix(obj->basematrix);  
    /* obj->query is linked in */ 
    /* obj->target is linked in */ 
    /* obj->comp is linked in */ 
    /* obj->gap is linked in */ 
    /* obj->ext is linked in */ 


    ckfree(obj); 
    return NULL; 
}    





#ifdef _cplusplus
}
#endif
