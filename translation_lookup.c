#include "cyc_core.h"
#include "cyc_render.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* BULK_DIALOGUE_V3: presentation only; no guest writes. */
#define LOOKUP_ROOT "./"
#define MAX_ENTRIES 4096u
#define MAX_PARTS 8u
#define MAX_SOURCE 512u
#define NO_SECOND UINT32_C(0xFFFFFFFF)
typedef struct { uint32_t ids[MAX_PARTS]; unsigned count; char line1[37], line2[37]; } Translation;
static Translation catalog[MAX_ENTRIES];
static unsigned catalog_count, active_count, active_valid, active_glyphs;
static uint32_t active_ids[MAX_PARTS];
static uint8_t expected[30*32], used[30*32];
static unsigned end_x, end_y;
static uint64_t reported[4096];
static unsigned reported_count;

static void coverage(const char *reason)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for (unsigned i=0;i<active_count;i++) for(unsigned j=0;j<4;j++) {
        h^=(active_ids[i]>>(j*8))&255u; h*=UINT64_C(1099511628211);
    }
    for(const char *p=reason;*p;p++){h^=(unsigned char)*p;h*=UINT64_C(1099511628211);}
    for(unsigned i=0;i<reported_count;i++) if(reported[i]==h)return;
    if(reported_count>=4096)return;
    reported[reported_count++]=h;
    FILE *f=fopen(LOOKUP_ROOT "translation_coverage.tsv","a");
    if(!f)return;
    for(unsigned i=0;i<active_count;i++)fprintf(f,"%s%05X",i?"+":"",(unsigned)active_ids[i]);
    fprintf(f,"\t%s\t%u\n",reason,active_glyphs);fclose(f);
}
static int parse_id(const char *s,uint32_t *id)
{
    size_t n=strlen(s);
    if(!n||n>5||strspn(s,"0123456789ABCDEFabcdef")!=n)return 0;
    unsigned long value=strtoul(s,NULL,16);
    if(value>=0x20000ul)return 0;
    *id=(uint32_t)value;return 1;
}
static int parse_key(char *first,const char *second,Translation *t)
{
    t->count=0;
    char *p=first;
    for(;;){
        char *next=strchr(p,'+');if(next)*next=0;
        if(t->count==MAX_PARTS||!parse_id(p,&t->ids[t->count]))return 0;
        t->count++;
        if(!next)break;
        p=next+1;
    }
    if(strcmp(second,"-")){
        if(t->count!=1||!parse_id(second,&t->ids[1]))return 0;
        t->count=2;
    }
    return 1;
}
static int same_key(const Translation *a,const Translation *b)
{
    return a->count==b->count&&!memcmp(a->ids,b->ids,a->count*sizeof(a->ids[0]));
}
static int supported_line(const char *s)
{
    /* DIALOGUE_DIGITS_V1 */
    if(strlen(s)>36)return 0;
    for(;*s;s++)
        if(!(*s>='A'&&*s<='Z')&&!(*s>='0'&&*s<='9')&&!strchr(" .,?[]",*s))return 0;
    return 1;
}

void english_lookup_reset(void)
{
    active_count=active_valid=active_glyphs=catalog_count=reported_count=0;
    memset(used,0,sizeof used);
    FILE *log=fopen(LOOKUP_ROOT "translation_catalog_status.log","w");
    FILE *f=fopen(LOOKUP_ROOT "dialogue_translations.tsv","r");
    if(!f){if(log){fprintf(log,"STOP: catalog could not be opened.\n");fclose(log);}return;}
    char line[512];unsigned number=0,rejected=0;
    while(fgets(line,sizeof line,f)){
        number++;
        if(!strchr(line,'\n')&&!feof(f)){int ch;while((ch=fgetc(f))!='\n'&&ch!=EOF){} rejected++;continue;}
        line[strcspn(line,"\r\n")]=0;
        if(!line[0]||line[0]=='#')continue;
        char *field[4]={line,NULL,NULL,NULL};int ok=1;
        for(unsigned i=1;i<4;i++){char *tab=strchr(field[i-1],'\t');if(!tab){ok=0;break;}*tab=0;field[i]=tab+1;}
        Translation t;
        if(!ok||strchr(field[3],'\t')||!parse_key(field[0],field[1],&t)||!field[2][0]||
           !supported_line(field[2])||!supported_line(field[3])){
            rejected++;if(log)fprintf(log,"REJECTED line %u: invalid fields or unsupported text.\n",number);continue;
        }
        int duplicate=0;
        for(unsigned i=0;i<catalog_count;i++)if(same_key(&catalog[i],&t))duplicate=1;
        if(duplicate||catalog_count==MAX_ENTRIES){rejected++;continue;}
        strcpy(t.line1,field[2]);strcpy(t.line2,field[3]);catalog[catalog_count++]=t;
    }
    fclose(f);if(log){fprintf(log,"LOADED %u entries; REJECTED %u entries.\n",catalog_count,rejected);fclose(log);}
}
static int source_byte(uint16_t pointer,unsigned *offset,uint8_t *b)
{
    if(*offset>=MAX_SOURCE||(unsigned)pointer+*offset>=0x10000u)return 0;
    if(!cyc_debug_peek((uint16_t)(pointer+*offset),b))return 0;
    (*offset)++;return 1;
}
void english_lookup_observe(uint32_t id,uint16_t pointer,int append)
{
    if(!append){
        active_count=active_glyphs=0;active_valid=1;memset(used,0,sizeof used);
    }else if(!active_valid)return;
    if(active_count==MAX_PARTS){active_valid=0;coverage("too_many_parts");return;}
    active_ids[active_count++]=id;
    if(id>=0x20000u||pointer<0x8000){active_valid=0;coverage("invalid_source");return;}
    const uint8_t *ram=cyc_cpu_ram();
    if(!ram){active_valid=0;coverage("missing_ram");return;}
    unsigned x=ram[0x37],y=ram[0x38],offset=0;uint8_t b;
    while(source_byte(pointer,&offset,&b)){
        if(b==0xFF){end_x=x;end_y=y;return;}
        if(b==0xFE){
            uint8_t nx,ny;
            if(!source_byte(pointer,&offset,&nx)||!source_byte(pointer,&offset,&ny))break;
            x=nx;y=ny;continue;
        }
        /* Only the known lower dialogue band. Other layouts keep native text. */
        if(x<3||x>=32||y<25||y>28){active_valid=0;coverage("outside_dialogue_band");return;}
        unsigned tile=(b&0x7Fu)+0x80u;if(tile>=0xC0u)tile+=6;
        if(tile>255){active_valid=0;coverage("unsupported_glyph");return;}
        unsigned a=y*32+x,above=a-32;
        expected[a]=(uint8_t)tile;used[a]=1;
        if(b&0x80u){expected[above]=0xF4;used[above]=1;}
        else if(!used[above]){expected[above]=0xFF;used[above]=1;}
        x++;active_glyphs++;
    }
    active_valid=0;coverage("unterminated_or_source_limit");
}
int english_lookup_lines(const char **line1,const char **line2)
{
    *line1=*line2=NULL;
    if(!active_valid||!active_count||!active_glyphs)return 0;
    const Translation *found=NULL;
    for(unsigned i=0;i<catalog_count;i++)if(catalog[i].count==active_count&&
       !memcmp(catalog[i].ids,active_ids,active_count*sizeof(active_ids[0]))){found=&catalog[i];break;}
    if(!found){coverage("missing_catalog_key");return 0;}
    for(int y=193;y<233;y++)if(cyc_render_line_bg_table(y)!=0x1000||
       cyc_render_line_scroll_x(y)!=16||cyc_render_line_scroll_y(y)!=239+y){coverage("layout_not_supported");return 0;}
    for(unsigned a=0;a<30*32;a++)if(used[a]&&cyc_render_nametable((uint16_t)(0x2000+a))!=expected[a]){
        coverage("native_text_not_matching");return 0;
    }
    if(end_x<32&&end_y<30&&!used[end_y*32+end_x]&&
       cyc_render_nametable((uint16_t)(0x2000+end_y*32+end_x))!=0xFF){coverage("native_tail_not_blank");return 0;}
    *line1=found->line1;*line2=found->line2;return 1;
}
