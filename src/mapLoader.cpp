#include "mapLoader.h"

// methods

TMX::Parser* mapLoader::tmx = new TMX::Parser();
TSX::Parser* mapLoader::tsx = new TSX::Parser();

mapLoader::mapLoader(std::string filePath, std::string basicName, std::string extension):
    m_filePath(filePath), m_basicName(basicName), m_extension(extension), m_currentLevel(-1) {

}

mapLoader::~mapLoader() {

}

Map * mapLoader::nextLevel() {
    printf("nextLevel\n");
    m_currentLevel++;
    std::string currentLevelName = m_basicName + std::to_string(m_currentLevel) + m_extension;
    return readFromFile(m_filePath, currentLevelName, 21, 21);
}

// static functions

Map * mapLoader::readFromFile(std::string filePath, std::string fileName, int mapWidth, int mapHeight) {
    std::cout << "loading file: " << filePath + fileName << std::endl;
    tmx->load( (filePath + fileName).c_str() );
    int width = tmx->mapInfo.width;
    int height = tmx->mapInfo.height;
    int beginX  = stoi(tmx->mapInfo.property.find("beginX")->second);
    int beginY  = stoi(tmx->mapInfo.property.find("beginY")->second);
    int endX    = stoi(tmx->mapInfo.property.find("endX" )->second);
    int endY    = stoi(tmx->mapInfo.property.find("endY" )->second);
    float scl = (float) tmx->mapInfo.tileWidth;

    int maxSize = width/3;

    std::vector<Pattern*> patterns[maxSize];

    for( std::map<std::string, TMX::Parser::TileLayer>::iterator it = tmx->tileLayer.begin(); it != tmx->tileLayer.end(); ++it) {
        patterns[stoi(it->second.property["size"])/3 - 1].push_back(new Pattern(stoi(it->second.property["size"]), width,it->second.data.contents));
    }
//////////////////////
    std::string mapStr = tmx->tileLayer[tmx->tileLayer.begin()->first].data.contents;
    std::string texStr = filePath + tmx->tilesetList[0].source;
////////////////////////
    tsx->load( texStr.c_str() );

    GLuint* textures = (GLuint*) calloc(255, sizeof(GLuint));
    bool* isWall = (bool*) calloc(255, sizeof(bool));

    for(std::vector<TSX::Parser::Tile>::iterator it = tsx->tileList.begin(); it != tsx->tileList.end(); ++it) {
        textures[it->id] = loadTexture((filePath + it->image.source).c_str());
        isWall[it->id] = it->property.find(std::string("Wall"))->second == std::string("true");
    }

    isWall[0] = false;
    // finished reading files, generating terrain

    mapStruct* tiles = new mapStruct[mapWidth * mapHeight];

    srand(time(NULL));

    for(int y = 0; y < mapHeight; y++) {
        for(int x = 0; x < mapWidth; x++) {
            tiles[y*mapWidth + x].id = 0;
        }
    }

    std::vector<Pattern*>* potentials = new std::vector<Pattern*>;
    for(int size = maxSize - 1; size >= 0; size--) {
        if(patterns[size].size() != 0) {
            for(int y = 0; y < mapHeight; y+=3) {
                for(int x = 0; x < mapWidth; x+=3) {
                    if(isAvailable(x, y, mapWidth, mapHeight, size*3+3, tiles)) {
                        for(unsigned int k = 0; k < patterns[size].size() ; k++) {
                            if(isConnecting(x, y, mapWidth, mapHeight, size*3+3, tiles, isWall, patterns[size].at(k))) {
                                potentials->push_back(patterns[size].at(k));
                            }
                        }
                        if(potentials->size() > 0){
                            Pattern* pattern = potentials->at(rand() % potentials->size());
                            for(int i = 0; i < size*3+3; i++) {
                                for(int j = 0; j < size*3+3; j++) {
                                    tiles[(y+j) * mapWidth + x + i].id = pattern->getType(i, j);
                                }
                            }
                            potentials->clear();
                        }
                    }
                }
            }
        }
    }

    for(int j = 0; j < mapHeight; j++) {
        for(int i = 0; i < mapWidth; i++) {
            printf("%d", tiles[j*mapWidth + i].id);
        }
        printf("\n");
    }

    Map * map = new Map(width, height, mapStr.c_str(), scl, beginX, beginY, endX, endY);

    map->initTextures(textures);

    return map;
}

bool mapLoader::isAvailable(int x, int y, int width, int height, int size, mapStruct* tiles) {
    if(x + size > width || y + size > height)
        return false;

    for(int j = y; j < y + size; j++) {
        for(int i = x; i < x + size; i++) {
            if(tiles[j*width + i].id != 0)
                return false;
        }
    }
    return true;
}

//need cleaning
bool mapLoader::isConnecting(int x, int y, int width, int height, int size, mapStruct* tiles, bool* isWall, Pattern* pattern) {
    for(int i = 1; i < size; i+=3) {
        if(y == 0){
            if(!isWall[pattern->getType(i, 0)])
                return false;
        }
        else {
            if(tiles[(y - 1)*width + x + i].id != 0 && isWall[pattern->getType(i, 0)] != isWall[tiles[(y - 1)*width + x + i].id] )
                return false;
        }
    }

    for(int i = 1; i < size; i+=3) {
        if(y == height - size ){
            if(!isWall[pattern->getType(i, size-1)])
                return false;
        }
        else {
            if(tiles[(y + size)*width + x + i].id != 0 && isWall[pattern->getType(i, size-1)] != isWall[tiles[(y + size)*width + x + i].id] )
                return false;
        }
    }

    for(int j = 1; j < size; j+=3) {
        if(x == 0 ){
            if(!isWall[pattern->getType(0, j)])
                return false;
        }
        else {
            if(tiles[(y + j)*width + x - 1].id != 0 && isWall[pattern->getType(0, j)] != isWall[tiles[(y + j)*width + x - 1].id] )
                return false;
        }
    }

    for(int j = 1; j < size; j+=3) {
        if(x == width - size ){
            if(!isWall[pattern->getType(size-1, j)])
                return false;
        }
        else {
            if(tiles[(y + j)*width + x + size].id != 0 && isWall[pattern->getType(size-1, j)] != isWall[tiles[(y + j)*width + x + size].id] )
                return false;
        }
    }

    return true;
}
