#include "mapLoader.h"

// methods

float chances[5] = {
    1.f/5.f * 3.f/3.f,
    1.f/5.f * 3.f/6.f,
    1.f/5.f * 3.f/9.f,
    1.f/5.f * 3.f/12.f,
    1.f/5.f * 3.f/15.f
    };

TMX::Parser* mapLoader::tmx = new TMX::Parser();
TSX::Parser* mapLoader::tsx = new TSX::Parser();

mapLoader::mapLoader(std::string filePath, std::string basicName, std::string extension):
    m_filePath(filePath), m_basicName(basicName), m_extension(extension), m_currentLevel(-1) {

        
    srand(time(NULL));

}

mapLoader::~mapLoader() {

}

Map * mapLoader::nextLevel() {
    printf("nextLevel\n");
    m_currentLevel++;
    std::string currentLevelName = m_basicName + std::to_string(m_currentLevel) + m_extension;
    return readFromFile(m_filePath, currentLevelName, 51, 51);
}

// static functions

Map * mapLoader::readFromFile(std::string filePath, std::string fileName, int mapWidth, int mapHeight) {
    std::cout << "loading file: " << filePath + fileName << std::endl;
    tmx->load( (filePath + fileName).c_str() );
    int width = tmx->mapInfo.width;
    int height = tmx->mapInfo.height;
    int beginX  = 1; //stoi(tmx->mapInfo.property.find("beginX")->second);
    int beginY  = 1; //stoi(tmx->mapInfo.property.find("beginY")->second);
    int endX    = 0; //stoi(tmx->mapInfo.property.find("endX" )->second);
    int endY    = 0; //stoi(tmx->mapInfo.property.find("endY" )->second);
    float scl = (float) tmx->mapInfo.tileWidth;

    int maxSize = width/3;

    std::vector<Pattern*> patterns[maxSize];

    std::vector<int> mapStr;
    std::vector<int> rotStr;
    std::string texStr;

    for( std::map<std::string, TMX::Parser::TileLayer>::iterator it = tmx->tileLayer.begin(); it != tmx->tileLayer.end(); ++it) {
        patterns[stoi(it->second.property["size"])/3 - 1].push_back(new Pattern(stoi(it->second.property["size"]), width, it->second.data.contents, 0));
        patterns[stoi(it->second.property["size"])/3 - 1].push_back(new Pattern(stoi(it->second.property["size"]), width, it->second.data.contents, 1));
        patterns[stoi(it->second.property["size"])/3 - 1].push_back(new Pattern(stoi(it->second.property["size"]), width, it->second.data.contents, 2));
        patterns[stoi(it->second.property["size"])/3 - 1].push_back(new Pattern(stoi(it->second.property["size"]), width, it->second.data.contents, 3));
    }
//////////////////////
    texStr = filePath + tmx->tilesetList[0].source;
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


    for(int y = 0; y < mapHeight; y++) {
        for(int x = 0; x < mapWidth; x++) {
            tiles[y*mapWidth + x].id = 0;
            tiles[y*mapWidth + x].rotation = 0;
            tiles[y*mapWidth + x].up    = false;
            tiles[y*mapWidth + x].down  = false;
            tiles[y*mapWidth + x].left  = false;
            tiles[y*mapWidth + x].right = false;
        }
    }

    std::vector<Pattern*>* potentials = new std::vector<Pattern*>;
    for(int size = maxSize - 1; size >= 1; size--) {
        if(patterns[size].size() != 0) {
            for(int y = 0; y < mapHeight; y+=3) {
                for(int x = 0; x < mapWidth; x+=3) {
                    float randNumber = rand() % 10000 / 10000.f;
                    bool tryPattern = randNumber < chances[size];
                    if(tryPattern && isAvailable(x, y, mapWidth, mapHeight, size*3+3, tiles)) {
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
                                    tiles[(y+j) * mapWidth + x + i].rotation = pattern->getRotation();
                                }
                            }
                            potentials->clear();
                        }
                    }
                }
            }
        }
    }

    bool* possible = (bool*) calloc(4, sizeof(bool));

    bool complete;

    do{
        complete = true;
        int x = 0, y = 0;
        for(int yy = 1; yy < mapHeight; yy+=3) {
            for(int xx = 1; xx < mapWidth; xx+=3) {
                if(tiles[yy*mapWidth + xx].id == -1){
                    if(hasPossibilities(tiles, xx, yy, mapWidth, mapHeight, possible)){
                        complete = false;
                        x = xx;
                        y = yy;
                        xx = mapWidth;
                        yy = mapHeight;
                        printf("new -1 id at %d : %d\n", x, y);
                    } else {
                        tiles[yy*mapWidth + xx].id = -2;
                    }
                }
            }
        }
        if(complete)
            for(int yy = 1; yy < mapHeight; yy+=3) {
                for(int xx = 1; xx < mapWidth; xx+=3) {
                    if(tiles[yy*mapWidth + xx].id == 0) {
                        complete = false;
                        x = xx;
                        y = yy;
                        xx = mapWidth;
                        yy = mapHeight;
                        tiles[y*mapWidth + x].id = -1;
                        printf("new 0 id at %d : %d\n", x, y);
                    }
                }
            }
        if(tiles[y*mapWidth + x].id < 0) { 
            while(!complete && hasPossibilities(tiles, x, y, mapWidth, mapHeight, possible)) {
                
                completeSurroundingsConnections(tiles, x, y, mapWidth, mapHeight, isWall);

                tiles[y*mapWidth + x].id = -1;
                
                int choice;
                do {
                    choice = rand() % 4;
                } while(!possible[choice]);
                switch(choice){
                    case 0:
                        tiles[y*mapWidth + x].down = true;
                        tiles[(y+3)*mapWidth + x].up = true;
                        y+=3;
                        break;
                    case 1:
                        tiles[y*mapWidth + x].up = true;
                        tiles[(y-3)*mapWidth + x].down = true;
                        y-=3;
                        break;
                    case 2:
                        tiles[y*mapWidth + x].right = true;
                        tiles[y*mapWidth + x+3].left = true;
                        x+=3;
                        break;
                    case 3:
                        tiles[y*mapWidth + x].left = true;
                        tiles[y*mapWidth + x-3].right = true;
                        x-=3;
                        break;
                }
            }
            tiles[y*mapWidth + x].id = -2;
            completeSurroundingsConnections(tiles, x, y, mapWidth, mapHeight, isWall);
        }
    } while(!complete);

    for(int y = 1; y < mapHeight-1; y+=3) {
        for(int x = 1; x < mapWidth-1; x+=3) {
            if(tiles[y*mapWidth + x].id < 0){
                Pattern* pattern = patternMatch(tiles[y*mapWidth + x], patterns[0], isWall);
                for(int i = -1; i <= 1; i++){
                    for(int j = -1; j <= 1; j++) {
                        tiles[(y+j)*mapWidth + x + i].id = pattern->getType(i+1, j+1);
                        tiles[(y+j)*mapWidth + x + i].rotation = pattern->getRotation();
                    }
                }
            }
        }
    }

    
    mapStr.clear();
    rotStr.clear();

    bool** passage = (bool**) calloc(mapWidth, sizeof(bool*));
    
    for(int i = 0; i < mapWidth; i++) {
        passage[i] = (bool*) calloc(mapHeight, sizeof(bool));
        for(int j = 0; j < mapHeight; j++)
            passage[i][j] = false;
    }
    
    for(int j = 1; j < mapHeight; j+=3) {
        for(int i = 1; i < mapWidth; i+=3) {
            passage[i][j+1] = tiles[j*mapWidth + i].down;
            passage[i][j-1] = tiles[j*mapWidth + i].up;
            passage[i+1][j] = tiles[j*mapWidth + i].right;
            passage[i-1][j] = tiles[j*mapWidth + i].left;
        }
    }

    for(int j = 0; j < mapHeight; j++) {
        for(int i = 0; i < mapWidth; i++) {
            mapStr.push_back(tiles[j*mapWidth + i].id);
            rotStr.push_back(tiles[j*mapWidth + i].rotation);
        }
    }

    #ifdef DEBUG
        for(int j = 0; j < mapHeight; j++) {
            for(int i = 0; i < mapWidth; i++) {
                printf("%s", (isWall[tiles[j*mapWidth + i].id])? "#" : " ");
                printf("%s", (passage[i][j])? "@" : " ");
                if(i%3 == 2)
                    printf(" | ");
            }
            printf("\n");
            if(j%3 == 2){
                for(int i = 0; i < mapWidth; i++)
                    printf("---");
                printf("\n");
            }
        }
    #endif


    Map * map = new Map(mapWidth, mapHeight, mapStr, rotStr, isWall, scl/4, beginX, beginY, endX, endY);

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

// up down right left
bool mapLoader::hasPossibilities(mapStruct* tiles, int x, int y, int mapWidth, int mapHeight, bool* possible) {
    if(x<0 || x >= mapWidth || y < 0 || y >= mapHeight)
        return false;
    possible[1] = y-3 > 0 &&            tiles[(y-3)*mapWidth + x].id == 0;
    possible[0] = y+3 < mapHeight-1 &&  tiles[(y+3)*mapWidth + x].id == 0;
    possible[3] = x-3 > 0 &&            tiles[y*mapWidth + x-3].id == 0;
    possible[2] = x+3 < mapWidth-1 &&   tiles[y*mapWidth + x+3].id == 0;
    return possible[0] || possible[1] || possible[2] || possible[3];
}

void mapLoader::completeSurroundingsConnections(mapStruct* tiles, int x, int y, int mapWidth, int mapHeight, bool* isWall) {
    float connectProba = 0.1f;
    if(y-3 >= 0 && tiles[(y-3)*mapWidth + x].id <= 0 ) { // up
        float proba = rand() % 10000 / 10000.f;
        if(proba <= connectProba) {
            tiles[y*mapWidth + x].up = true;
            tiles[(y-3)*mapWidth + x].down = true;
        }
    } else if(y-2 >= 0 && tiles[(y-2)*mapWidth + x].id > 0 ) {
        tiles[y*mapWidth + x].up = !isWall[tiles[(y-2)*mapWidth + x].id];
    }
    
    if(y+3 < mapHeight && tiles[(y+3)*mapWidth + x].id <= 0 ) { // down
        float proba = rand() % 10000 / 10000.f;
        if(proba <= connectProba) {
            tiles[y*mapWidth + x].down = true;
            tiles[(y+3)*mapWidth + x].up = true;
        }
    } else if(y+2 < mapHeight && tiles[(y+2)*mapWidth + x].id > 0 ) {
        tiles[y*mapWidth + x].down = !isWall[tiles[(y+2)*mapWidth + x].id];
    }
    
    if(x-3 >= 0 && tiles[y*mapWidth + x-3].id <= 0 ) { // left
        float proba = rand() % 10000 / 10000.f;
        if(proba <= connectProba) {
            tiles[y*mapWidth + x].left = true;
            tiles[y*mapWidth + x-3].right = true;
        }
    } else if(x-2 >= 0 && tiles[y*mapWidth + x-2].id > 0 ) {
        tiles[y*mapWidth + x].left = !isWall[tiles[y*mapWidth + x-2].id];
    }
    
    if(x+3 < mapWidth && tiles[y*mapWidth + x+3].id <= 0 ) { // right
        float proba = rand() % 10000 / 10000.f;
        if(proba <= connectProba) {
            tiles[y*mapWidth + x].right = true;
            tiles[y*mapWidth + x+3].left = true;
        }
    } else if(x+2 < mapWidth && tiles[y*mapWidth + x+2].id > 0 ) {
        tiles[y*mapWidth + x].right = !isWall[tiles[y*mapWidth + x+2].id];
    }

}

Pattern* mapLoader::patternMatch(mapStruct tile, std::vector<Pattern*> patterns, bool* isWall) {
    for(Pattern* pattern : patterns) {
        bool result = true;
        result = result && (tile.up       == !isWall[pattern->getType(1, 0)]);
        result = result && (tile.down     == !isWall[pattern->getType(1, 2)]);
        result = result && (tile.right    == !isWall[pattern->getType(2, 1)]);
        result = result && (tile.left     == !isWall[pattern->getType(0, 1)]);
        if(result)
            return pattern;
    }
    return patterns[4];
}
