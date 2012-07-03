


#include "../include/fffCompiler.h"


_fff_NAMESPACE_PREFIX Compiler::Compiler(
	ClEnv &env,
	String includeDirectory,
	UInt lb2FftElementsCount,
	UInt kernelElementsCount)
	:
	m_includeDirectory(
		includeDirectory),
	m_lb2FftElementsCount(
		lb2FftElementsCount),
	m_kernelElementsCount(
		kernelElementsCount),
	m_env(
		env)
{
	StringStream source;
	source
		<< "#include \""
		<< "../cl/fffConvolution.cpp.cl"
		<< "\""
		<< std::endl;

	m_program = cl::Program(
		env.getContext(),
		source.str().c_str(),
		false
	);
}

cl::Kernel _fff_NAMESPACE_PREFIX Compiler::getKernel(
	String kernelName) const
{
	fff_RTCLC_ERR_INIT();

	cl::Kernel kernel;
	fff_RTCLC_SEQ_CHECK(
		kernel = cl::Kernel(m_program, kernelName.c_str(), fff_RTCLC_ERR_PTR));

	return kernel;
}

void _fff_NAMESPACE_PREFIX Compiler::build()
{
	fff_RTCLC_ERR_INIT();

	StringStream optionStream;
	optionStream << " -I " << m_includeDirectory << "";
	optionStream << " -D _fff_IS_DEV";
#ifdef fff_USE_64
	optionStream << " -D fff_USE_64";
#endif
	optionStream << " -D _fff_LB2_FFT_SIZE=" << getLb2FftElementsCount();
	optionStream << " -D _fff_KERNEL_SIZE=" << getKernelElementsCount();

	String buildOptions = optionStream.str();
	Devices devices(1);
	devices[0] = m_env.getDevice();

	Int buildErr;

	if((buildErr = m_program.build(devices, buildOptions.c_str())) != CL_SUCCESS)
	{
		//std::cout << buildErr;
		
		switch(buildErr)
		{
		case CL_BUILD_PROGRAM_FAILURE:
			Int buildInfoErr;

			fff_RTCLC_SEQ_CHECK(
				buildInfoErr = m_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(getEnv().getDevice(), fff_RTCLC_ERR_PTR));
			if(buildInfoErr != CL_SUCCESS)
			{
				String buildInfo;
				fff_RTCLC_SEQ_CHECK(
					buildInfo = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(getEnv().getDevice(), fff_RTCLC_ERR_PTR));

				if(buildInfo.size() > 500)
					buildInfo.resize(500);
				std::cout << buildInfo;
				fff_THROW(
					buildInfo.c_str());
			}
			break;
		default:
			fff_THROW(rtclcDebugErrorByName(buildErr));
			break;
		}
	}
}