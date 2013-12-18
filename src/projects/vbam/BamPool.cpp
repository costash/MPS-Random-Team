//===========================================================================
//===========================================================================
//===========================================================================
//==      BamPool.cpp                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"

#include <set>
#include <queue>
#include <algorithm>

#include "constants.h"
#include "BamPool.h"
#include "FileUtil.h"
#include "ImageInfo.h"
#include "Matrix.h"

#include "Util.hpp"
typedef unsigned int uint;

BamPool::BamPool(const TCHAR* bamsFolder, const TCHAR* inputImageName,
				 const TCHAR* outputFolder, const TCHAR* outputName)
				 : _bamsFolder(bamsFolder), _inputImageName(inputImageName),
				 _outputFolder(outputFolder), _outputName(outputName)
{
}

int BamPool::Init(const TCHAR* vbamExecutableName)
{
	int returnCode = FileUtil::GetFilesInDir(_bamsFolder.c_str(), _T(".exe"), _bamNames);
	if (returnCode == FileUtil::SUCCESS)
	{
		// Skip my executable if bams are in the current folder
#ifdef _DEBUG
#if 0
		_ftprintf_s(stderr, _T("bam names size before erase = %d\n"), _bamNames.size());
		for (unsigned int i = 0; i < _bamNames.size(); ++i)
		{
			_ftprintf_s(stderr, _T("%s "), _bamNames[i].c_str());
		}
		_ftprintf_s(stderr, _T("\n"));
#endif
#endif
		auto it = find(_bamNames.begin(), _bamNames.end(), std::wstring(vbamExecutableName));
		if (it != _bamNames.end())
		{
			_bamNames.erase(it);
		}
#ifdef _DEBUG
#if 0
		_ftprintf_s(stderr, _T("bam names size after erase = %d\n"), _bamNames.size());
		for (unsigned int i = 0; i < _bamNames.size(); ++i)
		{
			_ftprintf_s(stderr, _T("%s "), _bamNames[i].c_str());
		}
		_ftprintf_s(stderr, _T("\n"));
#endif
#endif
	}

	return returnCode;
}

void BamPool::SpawnAll(const TCHAR* processingTimeout, const TCHAR* initTimeout)
{
	unsigned int timeout = composeTimeout(processingTimeout, initTimeout);
	for (unsigned int i = 0; i < _bamNames.size(); ++i)
	{
		_bams[_bamNames[i]].reset(new Bam(_bamsFolder, _bamNames[i]));
		_bams[_bamNames[i]].get()->Run(_inputImageName, timeout);
	}
}

void BamPool::DummyVote()
{
	std::unique_ptr<Matrix<int>> zeroConfidences;
	std::unique_ptr<Matrix<int>> oneConfidences;
	int width = 0;
	int height = 0;
	std::map<std::wstring, std::unique_ptr<ImageInfo>> imageInfos;

	for (unsigned int i = 0; i < _bamNames.size(); ++i)
	{
		Bam* bam = _bams[_bamNames[i]].get();
		if (bam->LastRunStatus() != Bam::EXECUTED_SUCCESSFULLY)
		{
			continue;
		}

		std::unique_ptr<KImage> binarizedImage(new KImage(bam->ImagePath().c_str()));
		if (binarizedImage.get() == NULL || !binarizedImage.get()->IsValid() )
		{
			_tprintf(_T("File %s can't be read!"), bam->ImagePath().c_str());
			continue;
		}

		if (binarizedImage.get()->GetBPP() != 1)
		{
			_tprintf(_T("File %s is not a valid 1BPP image!"), bam->ImagePath().c_str());
			continue;
		}
		std::unique_ptr<KImage> confidenceImage(new KImage(bam->ConfPath().c_str()));
		if (confidenceImage.get() == NULL || !confidenceImage.get()->IsValid())
		{
			_tprintf(_T("File %s can't be read!"), bam->ConfPath().c_str());
			continue;
		}
		if (confidenceImage.get()->GetBPP() != 8)
		{
			_tprintf(_T("File %s is not a valid 8BPP image!"), bam->ConfPath().c_str());
			continue;
		}
		if (!binarizedImage.get()->BeginDirectAccess() || !confidenceImage->BeginDirectAccess())
		{
			_tprintf(_T("Files %s or %s could not begin direct access!"),
				bam->ImagePath(), bam->ConfPath().c_str());
			continue;
		}

		// Create the accumulator matrices
		if (i == 0)
		{
			width = confidenceImage.get()->GetWidth();
			height = confidenceImage.get()->GetHeight();
			zeroConfidences.reset(new Matrix<int>(width, height));
			oneConfidences.reset(new Matrix<int>(width, height));
		}

		// Compute additional info from image
		uint32_t pixelSum = 0;
		uint64_t confidenceSum = 0;
		for (int c = 0; c < height; ++c)
		{
			for (int r = 0; r < width; ++r)
			{
				pixelSum += binarizedImage.get()->Get1BPPPixel(r, c);
				confidenceSum += confidenceImage.get()->Get8BPPPixel(r, c);
			}
		}

		imageInfos[_bamNames[i]].reset(new ImageInfo(pixelSum, confidenceSum));

		for (int c = 0; c < height; ++c)
		{
			for (int r = 0; r < width; ++r)
			{
				if (binarizedImage.get()->Get1BPPPixel(r, c) == false)
				{
					int confAccum = confidenceImage.get()->Get8BPPPixel(r, c) + zeroConfidences.get()->Get(r, c);
					zeroConfidences.get()->Set(r, c, confAccum);
				}
				else
				{
					int confAccum = confidenceImage.get()->Get8BPPPixel(r, c) + oneConfidences.get()->Get(r, c);
					oneConfidences.get()->Set(r, c, confAccum);
				}
			}
		}

		binarizedImage.get()->EndDirectAccess();
		confidenceImage.get()->EndDirectAccess();
	}

	// Create the final output image
	std::unique_ptr<KImage> votedOutput(new KImage(width, height, 1));
	if (votedOutput.get()->BeginDirectAccess())
	{
		for (int c = 0; c < height; ++c)
		{
			for (int r = 0; r < width; ++r)
			{
				bool pixel = zeroConfidences.get()->Get(r, c) < oneConfidences.get()->Get(r, c);
				votedOutput.get()->Put1BPPPixel(r, c, pixel);
			}
		}
	}
	
	// Save output image
	if (!votedOutput.get()->SaveAs((_outputName + std::wstring(_T(".TIFF"))).c_str(), SAVE_TIFF_CCITTFAX4))
	{
		_tprintf(_T("Unable to save image: %s"), (_outputName + std::wstring(_T(".TIFF"))).c_str());
	}

	votedOutput.get()->EndDirectAccess();
}

static const int UNDEF_SIZE = -1;
static const float SEGM_CT  = 1.8f;
static const bool WHITE = false;
static const bool BLACK = true;

static const int DEF_DIM = 1024;
typedef struct Vertex{	
	Vertex(int x,int y) : x(x), y(y){};
	int x, y;
} Vertex;

class AlgoState{

public:

	int height;
	int width;

	int start_x;
	int start_y;
	int end_x;
	int end_y;
	int num_samples;
	
	std::vector< KImage* > BamImagePixels; 
	std::vector< KImage* > BamConf;
	Matrix<bool>*   ImagePixels;
	Matrix<float>*  ImageConf;

	std::vector< Vertex > uncertain_pixels;

	std::pair< int, int > underSegmBam;
	std::pair< int, int > overSegmBam;
};

static inline void setParam( int& param, int value, const char* message )
{
	if( param == UNDEF_SIZE ){
		param = value;
	}
	else{
		//_assert( param == value, message );
		DIE2( param == value, message, 1 );
	}
}

#define TRIM_ISOLATED_PIXELS
static const int dx[] = { 1,  1, 1, 0, 0,  -1, -1, -1 };
static const int dy[] = { -1, 0, 1, 1, -1, -1,  0,  1 };
static const int DIR_CNT  = 8;

static inline void b_heuristic( AlgoState& state, 
							    std::vector<float>& one_conf, 
								std::vector<float>& zero_conf )
{

	uint BamCount = state.BamConf.size();
	std::vector< int > variance( BamCount );
	
	for( uint k = 0; k < BamCount; ++k ){

		KImage* image_pixels = state.BamImagePixels[k];
		KImage* image_conf   = state.BamConf[k];

		for( int j = state.start_y; j < state.end_y; ++j ){	
			for( int i = state.start_x; i < state.end_x; ++i ){

				bool pixel = image_pixels->Get1BPPPixel(j,i);

#ifdef TRIM_ISOLATED_PIXELS
				int black = 0, white = 0; // count number of surrounding black/white pixels
				for( uint p = 0; p < DIR_CNT; ++p ){
					
					int nx = i + dx[p];
					int ny = j + dy[p];

					if( nx < 0 || ny < 0 || nx >= state.height || ny >= state.width ){
						break;
					}

					bool pixel = image_pixels->Get1BPPPixel(ny,nx);
					(pixel == BLACK) ? (++black) : (++white);
				}

				if( black == DIR_CNT && pixel == WHITE && image_conf->Get8BPPPixel(j,i) != 255 ){
					image_pixels->Put1BPPPixel( j,i, BLACK );
					image_conf->Put8BPPPixel( j,i, 255 );
				}
				
				if( white == DIR_CNT && pixel == BLACK && image_conf->Get8BPPPixel(j,i) != 255 ){
					image_pixels->Put1BPPPixel( j,i, WHITE );
					image_conf->Put8BPPPixel( j,i, 255 );
				}
#endif

				variance[k] += image_pixels->Get1BPPPixel(j,i) == BLACK;
			}
		}
	}

	state.underSegmBam.first = 0;
	state.underSegmBam.second = variance[0];

	state.overSegmBam.first = 0;
	state.overSegmBam.second = variance[0];

	for( uint k = 1; k < BamCount; ++k ){

		int var = variance[k];

		if( state.underSegmBam.second < var ){
			state.underSegmBam.first = k, state.underSegmBam.second = var;
		}

		if( state.overSegmBam.second > var ){
			state.overSegmBam.first = k, state.overSegmBam.second = var;
		}
	}

	/*
		Bam-uri busite
	*/

	int cOver = 0, cUnder = 0;
	for( uint k = 0; k < BamCount; ++k ){

		if( state.overSegmBam.second / 2 > (variance[k]) ){
			++cOver;
		}
	}

	if( cOver == BamCount-1 ){
		one_conf[ state.overSegmBam.first ] = zero_conf[ state.underSegmBam.first ] = 0.1f;
	}
	else{
		if( BamCount > 1 ){
			one_conf[ state.underSegmBam.first ] = SEGM_CT;
			zero_conf[ state.overSegmBam.first ] = SEGM_CT;
		}
	}
}

static void vote( AlgoState& state )
{
	uint BamCount = state.BamConf.size();
	std::vector< float > one_conf( BamCount, 1.0f );
	std::vector< float > zero_conf( BamCount, 1.0f );
	b_heuristic( state, one_conf, zero_conf );

	for( int j = state.start_y; j < state.end_y; ++j ){
		for( int i = state.start_x; i < state.end_x; ++i ){

			int b_pixel	 = 0, w_pixel = 0;
			float b_conf = 0.0f, w_conf = 0.0f;

			for( uint k = 0; k < BamCount; ++k ){

				KImage* image_pixels = state.BamImagePixels[k];
				KImage* image_conf   = state.BamConf[k];
				
				float conf = image_conf->Get8BPPPixel(j,i);
				if( conf < 128 ){
					conf = 128;
				}

				bool pixel = image_pixels->Get1BPPPixel(j,i);
				if( pixel == WHITE ){
					conf *= zero_conf[k];
					w_pixel++;
					w_conf += conf;
				}else{
					conf *= one_conf[k];
					b_pixel++;
					b_conf += conf;
				}
			}

			b_conf /= BamCount * 255;
			w_conf /= BamCount * 255;
			
			float f_conf;
			bool marked = false;

			if( b_pixel > w_pixel )
			{
				if( !w_pixel 
				//	|| state.BamImagePixels[state.underSegmBam.first]->Get1BPPPixel(j,i) == BLACK
				){
					state.ImagePixels->Set(j,i,BLACK);
					state.ImageConf->Set(j,i,1.0f);
					f_conf = 1.0f;
					marked = true;
				}
			}
			else {
				if( w_pixel > b_pixel )
				{
					if( !b_pixel 
					//	|| state.BamImagePixels[state.overSegmBam.first]->Get1BPPPixel(j,i) == WHITE
					){
						state.ImagePixels->Set(j,i,WHITE);
						state.ImageConf->Set(j,i,0.0f);
						f_conf = 0.0f;					
						marked = true;
					}
				}
			}

			if( !marked ){
				if( b_conf > w_conf ){
					state.ImagePixels->Set(j,i,BLACK);
					state.ImageConf->Set(j,i,b_conf);
					f_conf = b_conf;
				} else{
					state.ImagePixels->Set(j,i,WHITE);
					state.ImageConf->Set(j,i,1-w_conf);
					f_conf = 1-w_conf;
				}
			}
	
			if( f_conf != 0.0f && f_conf != 1.0f ){
				state.uncertain_pixels.push_back( Vertex(j,i) );
			}
		}
	}
}


static void splitImage( AlgoState& state, int num_samples )
{
	if( num_samples == 1 
	|| state.start_x >= state.end_x 
	|| state.start_y >= state.end_y  ){
		vote( state );
	}
	else{

		// split image in half
		// for a more accurate estimation of the most under/over segmenting algorithm
		int dx = (state.end_x - state.start_x ) >> 1;
		
		int sx = state.start_x, sy = state.start_y;
		int ex = state.end_x, ey = state.end_y;

		state.start_x = sx;		 state.start_y = sy;
		state.end_x   = sx + dx; state.end_y   = ey;
		splitImage( state, num_samples / 2 );

		state.start_x = sx + dx; state.start_y = sy;
		state.end_x   = ex; state.end_y = ey;
		splitImage( state, num_samples / 2 );
	}
}

/*
static const float boundary_ct = 1.0f;
static void graphCut( AlgoState& state )
{
	uint size = state.uncertain_pixels.size();
	std::set< std::pair<int,int> > marked;

	GraphFlow< float, DEF_DIM > graph;

	for( uint i = 0; i < size; ++i ){

		std::pair< int, int > pos = std::make_pair(state.uncertain_pixels[i].x, state.uncertain_pixels[i].y);

		if( marked.find( pos ) != marked.end() ){
			continue;
		}
		marked.insert(pos);


		int idx_counter = 0;
		std::queue< Vertex* > queue;
		std::vector< Vertex > nodes;
		nodes.push_back( state.uncertain_pixels[i] );
		queue.push( &state.uncertain_pixels[i] );

		while( !queue.empty() )
		{
			Vertex* current = queue.front();
			queue.pop();

			float conf = state.ImageConf->Get( current->x, current->y );
			if( conf == 0.0f || conf == 1.0f ){
				continue;
			}
			
			int c_idx = idx_counter;

			for( uint p = 0; p < DIR_CNT; ++p ){
					
				int nx = current->x + dx[p];
				int ny = current->y + dy[p];

				if( nx < 0 || ny < 0 || nx >= state.height || ny >= state.width ){
					break;
				}
				
				nodes.push_back( Vertex(nx,ny) );
				idx_counter++;

				
				float cap;
				float neigh_tendency = state.ImageConf->Get( current->x, current->y );
				if( neigh_tendency == 0.0f ){
					cap = 1 - conf;
				}
				else{
					if( neigh_tendency == 1.0f ){
						cap = conf;
					}
					else{
						if( neigh_tendency > conf ){
							cap = neigh_tendency - conf;
						}
						else{
							cap = conf - neigh_tendency;
						}
					}
				}

				graph.add_link( c_idx, idx_counter, cap * boundary_ct );
			}
		}

		int source = idx_counter++;
		int sink   = idx_counter++;

		graph.sink	 = sink;
		graph.source = source;

		int idx = 0;
		for( uint i = 0, sz = nodes.size(); i < sz; ++i ){
		
			Vertex& v = nodes[i];
			float prob = state.ImageConf->Get( v.x, v.y );

			graph.link_to_sink( idx, prob );
			graph.link_to_source( idx, 1 - prob );

			++idx; 
		}

		graph.MaxFlow();
		std::vector< int > cut = graph.MinCut();

		for( uint i = 0, sz = nodes.size(); i < sz; ++i ){
			state.ImagePixels->Set( nodes[i].x, nodes[i].y, WHITE );
		}

		for( uint i = 0, sz = cut.size(); i < sz; ++i ){
			state.ImagePixels->Set( nodes[ cut[i] ].y, nodes[ cut[i] ].x, BLACK );
		}

		graph.reset();
	}
}*/

void BamPool::SmartVote( int num_samples )
{
	AlgoState state;
	state.height = -1, state.width = -1;
	
	std::vector< KImage* >& BamImagePixels = state.BamImagePixels;
	std::vector< KImage* >& BamConf		  = state.BamConf;

	for( unsigned int i = 0, sz = _bamNames.size(); i < sz; ++i )
	{
		Bam* bam = _bams[ _bamNames[i] ].get();
		DIE2( bam->LastRunStatus() == Bam::EXECUTED_SUCCESSFULLY, "Bam Execution fail", 1);

		KImage* image_pixels = new KImage( bam->ImagePath().c_str() );
		DIE2( image_pixels != NULL && image_pixels->IsValid(), "File can't be read!",  1);   
		DIE2( image_pixels->GetBPP() == 1, "File is not a valid 1BPP image!", 1 );

		KImage* image_conf = new KImage( bam->ConfPath().c_str() );
		DIE2( image_conf != NULL && image_conf->IsValid(), "File can't be read!", 1);   
		DIE2( image_conf->GetBPP() == 8, "File is not a valid 1BPP image!", 1 );
		DIE2( image_pixels->BeginDirectAccess() && image_conf->BeginDirectAccess(), "Files or could not begin direct access!", 1 );
		
		BamImagePixels.push_back( image_pixels );
		BamConf.push_back( image_conf );

		setParam( state.height, image_pixels->GetHeight(), "Invalid/Inconsistent height");
		setParam( state.width,  image_pixels->GetWidth(),  "Invalid/Inconsistent width");
	}

	Matrix<bool>  ImagePixels( state.width, state.height  );
	Matrix<float> ImageConf( state.width, state.height );

	for( int i = 0; i < state.height; ++i ){
		for( int j = 0; j < state.width; ++j ){
			ImagePixels.Set(j,i,0);
			ImageConf.Set(j,i,0);
		}
	}

	state.ImageConf   = &ImageConf;
	state.ImagePixels = &ImagePixels;

	state.start_x = 0, state.start_y = 0;
	state.end_x = state.height, state.end_y = state.width; 
	splitImage( state, num_samples );
	//graphCut( state );

	// Create the final output image
	KImage* votedOutput = new KImage(state.width, state.height, 1);
	DIE2( votedOutput->BeginDirectAccess(), "Failure accessing image", 1);

	for( int j = 0; j < state.width; ++j ){
		for( int i = 0; i < state.height; ++i ){
			votedOutput->Put1BPPPixel( j,i, state.ImagePixels->Get(j,i) );
		}
	}

	BOOL ret = votedOutput->SaveAs((_outputName + std::wstring(_T(".TIFF"))).c_str(), SAVE_TIFF_CCITTFAX4);
	DIE2( ret != 0, "Unable to save confidence image:", 1 );
	DIE2( !votedOutput->EndDirectAccess(), "Fail closing output image\n", 1);

	for( unsigned int i = 0, size = BamImagePixels.size(); i < size; ++i ){
		BamImagePixels[i]->EndDirectAccess();
		BamConf[i]->EndDirectAccess();
	}

	delete votedOutput;
}


unsigned int BamPool::composeTimeout(const TCHAR* processingTimeout, const TCHAR* initTimeout)
{
	std::wstring processing(processingTimeout);
	std::wstring init(initTimeout);

	float processingMilisec  = std::stof(processingTimeout);
	unsigned int initMilisec = std::stoul(initTimeout);

	std::unique_ptr<KImage> input(new KImage(_inputImageName.c_str()));
	int pixels = input.get()->GetWidth() * input.get()->GetHeight();

	return (unsigned int) (pixels * processingMilisec) + initMilisec;
}
