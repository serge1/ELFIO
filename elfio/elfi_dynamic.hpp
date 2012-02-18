#ifndef ELFI_DYNAMIC_HPP
#define ELFI_DYNAMIC_HPP

class ELFIDynamicReader : virtual public ELFIReaderImpl, virtual public IELFIDynamicReader
{
  public:
    ELFIDynamicReader( const IELFI* pIELFI, const section* pSection );
    virtual ~ELFIDynamicReader();

    // Dynamic reader functions
    virtual Elf_Xword  getEntriesNum() const;
    virtual ELFIO_Err  get_entry( Elf_Xword    index,
                                 Elf_Sxword& tag,
                                 Elf_Xword&  value ) const;
};


ELFIDynamicReader::ELFIDynamicReader( const IELFI* pIELFI, const section* pSection ) :
    ELFIReaderImpl( pIELFI, pSection )
{
}


ELFIDynamicReader::~ELFIDynamicReader()
{
}


Elf_Xword
ELFIDynamicReader::getEntriesNum() const
{
    Elf_Xword nRet = 0;
    if ( 0 != m_pSection->get_entry_size() ) {
        nRet = m_pSection->get_size() / m_pSection->get_entry_size();
    }
    
    return nRet;
}


ELFIO_Err
ELFIDynamicReader::get_entry( Elf_Xword    index,
                             Elf_Sxword& tag,
                             Elf_Xword&  value ) const
{
    if ( index >= getEntriesNum() ) {    // Is index valid
        return ERR_ELFIO_INDEX_ERROR;
    }
    
    const Elf64_Dyn* pEntry = reinterpret_cast<const Elf64_Dyn*>(
            m_pSection->get_data() + index * m_pSection->get_entry_size() );
    tag   = convert2host( pEntry->d_tag, m_pIELFI->get_encoding() );
    value = convert2host( pEntry->d_un.d_val, m_pIELFI->get_encoding() );
    
    return ERR_ELFIO_NO_ERROR;
}

#endif // ELFI_DYNAMIC_HPP
