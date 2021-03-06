/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   this code is used to find the neighbors of channel elements (square lattice)
------------------------------------------------------------------------- */

#include "string.h"
#include "math.h"
#include "bond_channel_channel.h"
#include "group.h"
#include "modify.h"
#include "error.h"
#include "atom.h"
#include "atom_vec.h"
#include "force.h"
#include "stdlib.h" 
#include "math.h" 
#include "mpi.h"
#include "comm.h"
#include "update.h"
#include "neighbor.h"
#include "memory.h"
#include "domain.h"
#include "lattice.h"
#define TINY  1.e-3 ;
#define FAKE_INT_VALUE  -991 ;

#include "types.h"

//const int ROCK_ATOM_TYPE = 1;
//const int CHANNEL_ATOM_TYPE = 2;


using namespace LAMMPS_NS;


/* ---------------------------------------------------------------------- */

BondChannelChannel::BondChannelChannel(LAMMPS *lmp) : Pointers(lmp) {}

void BondChannelChannel::command(int narg, char **arg)
{

  //  if (narg < 3) error->all(FLERR,"Illegal injection_ini command"); // ID group-ID fixchannelini
 

  int nmax =0;
  int nall = atom->nlocal + atom->nghost;

  int n, m,i;
  int *atype = atom->type;
  int *tag = atom->tag;
  int channel_atomi, channel_atomj;
  double xi,yi,zi;
  double xj,yj,zj;
  double **x0 = atom->x0;
  int nlocal = atom->nlocal;
  int Ctype = 0;   // Ctype means channel type.
  int cond0,cond1,cond2,cond3,cond4,cond5;
  int cond6,cond7,cond8,cond9,cond10,cond11;

  int **bond_channel_atom = atom->bond_channel_atom;
  int *num_bond_channel_atom = atom->num_bond_channel_atom;

  int ii;
  int max_bond_num;

  double  lattice_spacing = domain->lattice->xlattice;  // It's correct only for cubic lattice, xlattice = ylattice = zlattice


  partner0 = NULL;
  partner1 = NULL;
  partner2 = NULL;
  partner3 = NULL;
  partner4 = NULL;
  partner5 = NULL;

  partner6 = NULL;
  partner7 = NULL;
  partner8 = NULL;
  partner9 = NULL;
  partner10 = NULL;
  partner11 = NULL;



  if (atom->nmax > nmax) {

    memory->destroy(partner0);
    memory->destroy(partner1);
    memory->destroy(partner2);
    memory->destroy(partner3);
    memory->destroy(partner4);
    memory->destroy(partner5);

    memory->destroy(partner6);
    memory->destroy(partner7);
    memory->destroy(partner8);
    memory->destroy(partner9);
    memory->destroy(partner10);
    memory->destroy(partner11);


    nmax = atom->nmax;

    memory->create(partner0,nmax,"bond_channel_atom:partner");
    memory->create(partner1,nmax,"bond_channel_atom:partner");
    memory->create(partner2,nmax,"bond_channel_atom:partner");
    memory->create(partner3,nmax,"bond_channel_atom:partner");
    memory->create(partner4,nmax,"bond_channel_atom:partner");
    memory->create(partner5,nmax,"bond_channel_atom:partner");

    memory->create(partner6,nmax,"bond_channel_atom:partner");
    memory->create(partner7,nmax,"bond_channel_atom:partner");
    memory->create(partner8,nmax,"bond_channel_atom:partner");
    memory->create(partner9,nmax,"bond_channel_atom:partner");
    memory->create(partner10,nmax,"bond_channel_atom:partner");
    memory->create(partner11,nmax,"bond_channel_atom:partner");

  }


  for (i = 0; i < nall; i++) {
    partner0[i] = FAKE_INT_VALUE;
    partner1[i] = FAKE_INT_VALUE;
    partner2[i] = FAKE_INT_VALUE;
    partner3[i] = FAKE_INT_VALUE;
    partner4[i] = FAKE_INT_VALUE;
    partner5[i] = FAKE_INT_VALUE;

    partner6[i] = FAKE_INT_VALUE;
    partner7[i] = FAKE_INT_VALUE;
    partner8[i] = FAKE_INT_VALUE;
    partner9[i] = FAKE_INT_VALUE;
    partner10[i] = FAKE_INT_VALUE;
    partner11[i] = FAKE_INT_VALUE;


  }
  
  
  for (n = 0; n < nlocal; n++){
    if (atype[n] != CHANNEL_ATOM_TYPE ) continue; // this is not a channel atom
    
    channel_atomi = n;
    num_bond_channel_atom[n]=12;
 
  
    for (m =0; m <nlocal+atom->nghost ; m++){
      if (atype[m] != CHANNEL_ATOM_TYPE ) continue; // this is not a channel atom
      channel_atomj =m;
      
      xi = x0[channel_atomi][0];
      yi = x0[channel_atomi][1];
      zi = x0[channel_atomi][2];
      
      xj = x0[channel_atomj][0];
      yj = x0[channel_atomj][1];
      zj = x0[channel_atomj][2];
      
      if ( fabs(fmod(xi,1.0*lattice_spacing)-0.5*lattice_spacing) < 1.e-6) {Ctype = 1;}
      else if ( fabs(fmod(yi,1.0*lattice_spacing)-0.5*lattice_spacing) < 1.e-6) {Ctype = 2;}
      else if ( fabs(fmod(zi,1.0*lattice_spacing)-0.5*lattice_spacing) < 1.e-6) {Ctype = 3;}
      else {Ctype = -FAKE_INT_VALUE; fprintf(screen, "Something Wrong!!!\n");}

      cond0=0; cond1=0; cond2=0; cond3=0; cond4=0; cond5=0;
      cond6=0; cond7=0; cond8=0; cond9=0; cond10=0; cond11=0;

      //      if (tag[channel_atomi]==3) fprintf(screen, "Ctype %d x y z  %.1f %.1f %.1f \n",Ctype, x0[channel_atomi][0],x0[channel_atomi][1],x0[channel_atomi][2]);
      
      if (Ctype ==1){
	if ( fabs(zj -zi) < 1.e-6) {
	  cond0 = ( (fabs(xj - xi)<1.e-6) && (fabs(yj - (yi - 1.0*lattice_spacing)) < 1.e-6) );  // first neighbor 
	  cond1 = ( (fabs(xj - xi)<1.e-6) && (fabs(yj - (yi + 1.0*lattice_spacing)) < 1.e-6) );  // second neighbor 
	  cond2 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond3 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond4 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond5 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
	if ( fabs(yj -yi) < 1.e-6) {
	  cond6 = ( (fabs(xj - xi)<1.e-6) && (fabs(zj - (zi - 1.0*lattice_spacing)) < 1.e-6) ); 
	  cond7 = ( (fabs(xj - xi)<1.e-6) && (fabs(zj - (zi + 1.0*lattice_spacing)) < 1.e-6) ); 
	  cond8 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond9 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond10 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond11 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
      }
      
      if (Ctype ==2){
	if ( fabs(zj -zi) < 1.e-6 ) {
	  cond0 = ( (fabs(xj - (xi -1.0*lattice_spacing) )<1.e-6) && (fabs(yj - yi) < 1.e-6) );  // first neighbor 
	  cond1 = ( (fabs(xj - (xi +1.0*lattice_spacing) )<1.e-6) && (fabs(yj - yi) < 1.e-6) ); 
	  cond2 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond3 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond4 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond5 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(yj - (yi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
	if ( fabs(xj -xi) < 1.e-6) {
	  cond6 = ( (fabs(yj - yi)<1.e-6) && (fabs(zj - (zi - 1.0*lattice_spacing) ) < 1.e-6) ); 
	  cond7 = ( (fabs(yj - yi)<1.e-6) && (fabs(zj - (zi + 1.0*lattice_spacing) ) < 1.e-6) ); 
	  cond8 = ( (fabs(yj - (yi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond9 = ( (fabs(yj - (yi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond10 = ( (fabs(yj - (yi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond11 = ( (fabs(yj - (yi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
      }

      if (Ctype ==3){
	if ( fabs(xj -xi) < 1.e-6) {
	  cond0 = ( (fabs(yj - (yi -1.0*lattice_spacing) )<1.e-6) && (fabs(zj - zi) < 1.e-6) );  // first neighbor 
	  cond1 = ( (fabs(yj - (yi +1.0*lattice_spacing) )<1.e-6) && (fabs(zj - zi) < 1.e-6) ); 
	  cond2 = ( (fabs(yj - (yi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond3 = ( (fabs(yj - (yi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond4 = ( (fabs(yj - (yi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond5 = ( (fabs(yj - (yi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
	if ( fabs(yj -yi) < 1.e-6) {
	  cond6 = ( (fabs(xj - (xi -1.0*lattice_spacing) )<1.e-6) && (fabs(zj - zi) < 1.e-6) );  // first neighbor 
	  cond7 = ( (fabs(xj - (xi +1.0*lattice_spacing) )<1.e-6) && (fabs(zj - zi) < 1.e-6) ); 
	  cond8 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond9 = ( (fabs(xj - (xi - 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	  cond10 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi - 0.5*lattice_spacing) ) < 1.e-6) );
	  cond11 = ( (fabs(xj - (xi + 0.5*lattice_spacing) )<1.e-6) && (fabs(zj - (zi + 0.5*lattice_spacing) ) < 1.e-6) );
	}
      }

      if (cond0 == 1){partner0[channel_atomi]=tag[channel_atomj];}
      if (cond1 == 1){partner1[channel_atomi]=tag[channel_atomj];}
      if (cond2 == 1){partner2[channel_atomi]=tag[channel_atomj];}
      if (cond3 == 1){partner3[channel_atomi]=tag[channel_atomj];}
      if (cond4 == 1){partner4[channel_atomi]=tag[channel_atomj];}
      if (cond5 == 1){partner5[channel_atomi]=tag[channel_atomj];}
      if (cond6 == 1){partner6[channel_atomi]=tag[channel_atomj];}
      if (cond7 == 1){partner7[channel_atomi]=tag[channel_atomj];}
      if (cond8 == 1){partner8[channel_atomi]=tag[channel_atomj];}
      if (cond9 == 1){partner9[channel_atomi]=tag[channel_atomj];}
      if (cond10 == 1){partner10[channel_atomi]=tag[channel_atomj];}
      if (cond11 == 1){partner11[channel_atomi]=tag[channel_atomj];}
    }
  }
  
  
  for (n = 0; n < nlocal; n++){
    if (atype[n] != CHANNEL_ATOM_TYPE ) continue; // this is not a channel atom
    //    num_bond_channel_atom[channel_atomi] = 6; // total six neighboring channels
    channel_atomi = n;

    int s[12] ={partner0[channel_atomi],partner1[channel_atomi],partner2[channel_atomi],partner3[channel_atomi],partner4[channel_atomi],partner5[channel_atomi],\
		partner6[channel_atomi],partner7[channel_atomi],partner8[channel_atomi],partner9[channel_atomi],partner10[channel_atomi],partner11[channel_atomi] };
    /*
    fprintf(screen, "********bond channel atoms %d %d %d %d %d %d %d %d %d %d %d %d ,max_bond_num %d at x y z %f %f %f \n", \
	    s[0],s[1],s[2],s[3],s[4],s[5],\
	    s[6],s[7],s[8],s[9],s[10],s[11],\
	    num_bond_channel_atom[n], x0[n][0],x0[n][1],x0[n][2]);
    */

    int *p =sort(s,12);
	//    int p[6];
    //p = sort(s,6);
    //    fprintf(screen, "====sorting %d %d %d %d %d %d  max_bond_num %d \n",p[0],p[1],p[2],p[3],p[4],p[5],num_bond_channel_atom[channel_atomi]);
    bond_channel_atom[channel_atomi][0] = p[0];
    bond_channel_atom[channel_atomi][1] = p[1];
    bond_channel_atom[channel_atomi][2] = p[2];
    bond_channel_atom[channel_atomi][3] = p[3];
    bond_channel_atom[channel_atomi][4] = p[4];
    bond_channel_atom[channel_atomi][5] = p[5];

    bond_channel_atom[channel_atomi][6] = p[6];
    bond_channel_atom[channel_atomi][7] = p[7];
    bond_channel_atom[channel_atomi][8] = p[8];
    bond_channel_atom[channel_atomi][9] = p[9];
    bond_channel_atom[channel_atomi][10] = p[10];
    bond_channel_atom[channel_atomi][11] = p[11];


    
    for (ii = 1; ii < 12; ii++){
      if ( bond_channel_atom[channel_atomi][ii-1]>=0 && bond_channel_atom[channel_atomi][ii] < 0){
	num_bond_channel_atom[channel_atomi]= ii;
      }
    }
  
  //    max_bond_num=num_bond_channel_atom[channel_atomi];
    /*    
    for (ii = 0; ii< max_bond_num; ii++){
      bond_type[rubber_atomi][ii] =1;
    }
    */
  
  }
  //  comm->forward_comm();
}


int * BondChannelChannel::sort(int *s, int n){
  

  int ii, jj,temp;

  for(int ii=0; ii<n; ii++){
    for(int jj=0; jj<n-1; jj++){
      if(s[jj]<s[jj+1]){
	temp = s[jj+1];
	s[jj+1] = s[jj];
	s[jj] = temp;
      }
    }
  }
  return s;
}
