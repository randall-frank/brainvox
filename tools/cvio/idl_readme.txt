

IDL interface to the CVIO library:

Variables ending in "_" are outputs.

err = CVIO_GET_ERROR_MESSAGE(errval,msg_)
err = CVIO_LISTSHMEM(names_,num_names_)
err = CVIO_CREATE(name,nsamples,datatype,ndims,dims,flags)
err = CVIO_DELETE(filename)
err = CVIO_OPEN(filename,access,stream_)
err = CVIO_CLOSE(stream)
err = CVIO_HAS_WRITER(stream,nwriters_)
err = CVIO_MAX_LENGTH(stream,length_)
err = CVIO_ADD_SAMPLES(stream,time,buffer,count)
err = CVIO_READ_SAMPLES(stream,time_,buffer_,count) (time_ is input and output)
err = CVIO_GETATTRIBUTE(stream,attr,value_)
err = CVIO_SETATTRIBUTE(stream,attr[,value])  (no value removes the attr)
err = CVIO_GETATTRIBUTELIST(stream,names_,num_names_)
timestamp = CVIO_CURRENT_TIME()
err = CVIO_READ_NEXT_SAMPLES(stream,time_,buffer_,count_)
err = CVIO_TELL(stream,cur_sample_,cur_time_,num_samples_)
err = CVIO_SEEK(stream,is_time_flag,value)
err = CVIO_DATATYPE(stream,datatype_,num_dims_,dims_)

