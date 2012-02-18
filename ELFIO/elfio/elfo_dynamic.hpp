#ifndef ELFO_DYNAMIC_HPP
#define ELFO_DYNAMIC_HPP

// Dynamic section producer
class ELFODynamicWriter : public IELFODynamicWriter
{
  public:
    ELFODynamicWriter( IELFO* pIELFO, IELFOSection* pSection );
    ~ELFODynamicWriter();
  
    virtual int addRef();
    virtual int release();

    virtual ELFIO_Err addEntry( Elf_Sword tag, Elf_Word value );

  private:
    int           m_nRefCnt;
    IELFO*        m_pIELFO;
    IELFOSection* m_pSection;
};


ELFODynamicWriter::ELFODynamicWriter( IELFO* pIELFO, IELFOSection* pSection ) :
        m_nRefCnt( 1 ),
        m_pIELFO( pIELFO ),
        m_pSection( pSection )
{
    m_pIELFO->addRef();
    m_pSection->addRef();
}


ELFODynamicWriter::~ELFODynamicWriter()
{
}


int
ELFODynamicWriter::addRef()
{
    m_pIELFO->addRef();
    m_pSection->addRef();
    return ++m_nRefCnt;
}


int
ELFODynamicWriter::release()
{
    int nRet             = --m_nRefCnt;
    IELFO*        pIELFO = m_pIELFO;
    IELFOSection* pSec   = m_pSection;

    if ( 0 == m_nRefCnt ) {
        delete this;
    }
    pSec->release();
    pIELFO->release();

    return nRet;
}


ELFIO_Err
ELFODynamicWriter::addEntry( Elf_Sword tag, Elf_Word value )
{
    Elf64_Dyn entry;
    entry.d_tag      = convert2host( tag, m_pIELFO->get_encoding() );
    entry.d_un.d_val = convert2host( value, m_pIELFO->get_encoding() );

    return m_pSection->appendData( reinterpret_cast<const char*>( &entry ),
                                sizeof( entry ) );
}

#endif // ELFO_DYNAMIC_HPP
