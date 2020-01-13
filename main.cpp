#include <cstddef>
#include <initializer_list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <random>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <exception>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <dirent.h>

class Board {
public:
    static constexpr std::size_t nCol = 10;
    static constexpr std::size_t nRow = 20;
    inline Board();
    Board(const sf::Color m[][nCol]) { copy_matrix(matrix, m); };
    Board(const Board &b): Board(b.matrix) {}
    Board &operator=(const Board &b) { copy_matrix(matrix, b.matrix); return *this; }
    Board &paint(unsigned x, unsigned y, sf::Color c) { do_paint(x, y, c); return *this; }
    Board &paint(std::initializer_list<sf::Vector2u> points, sf::Color c);
    Board &paint(const std::vector<sf::Vector2u> &points, sf::Color c);
    void remove_line(unsigned y);
    bool filled(int x, int y) const;
    bool filled(sf::Vector2i pos) const { return filled(pos.x, pos.y); }
    void draw(sf::RenderWindow &window, sf::RectangleShape &square) const;
private:
    sf::Color matrix[nRow][nCol];
    void do_paint(unsigned x, unsigned y, sf::Color c) {matrix[y][x] = c;}
    static void copy_matrix(sf::Color m1[][nCol], const sf::Color m2[][nCol]);
};

void Board::copy_matrix(sf::Color m1[][nCol], const sf::Color m2[][nCol]) {
    for (std::size_t i = 0; i < nRow; ++i)
        for (std::size_t j = 0; j < nCol; ++j)
            m1[i][j] = m2[i][j];
}

Board::Board() {
    for (auto &r : matrix)
        for (auto &el : r)
            el = sf::Color::White;
}

inline Board &Board::paint(std::initializer_list<sf::Vector2u> points, sf::Color c) {
    for (const auto &v : points)
        do_paint(v.x, v.y, c);
    return *this;
}

inline Board &Board::paint(const std::vector<sf::Vector2u> &points, sf::Color c) {
    for (const auto &v : points)
        do_paint(v.x, v.y, c);
    return *this;
}

void Board::remove_line(unsigned y) {
    for (std::size_t i = y; i > 0; --i)
        for (std::size_t j = 0; j < nCol; ++j)
            matrix[i][j] = matrix[i-1][j];
    for (std::size_t j = 0; j < nCol; ++j)
        matrix[0][j] = sf::Color::White;
}


inline bool Board::filled(int x, int y) const {
    return x < 0 || x >= nCol || y < 0 || y >= nRow || matrix[y][x] != sf::Color::White;
}

void Board::draw(sf::RenderWindow &window, sf::RectangleShape &square) const {
    const float sqWidth = square.getSize().x, sqHeigth = square.getSize().y;
    sf::Vector2f pos = square.getPosition();
    for (std::size_t i = 0; i < nCol; ++i, square.move(sqWidth, -sqHeigth * nRow))
        for (std::size_t j = 0; j < nRow; ++j, square.move(0.0f, sqHeigth)) {
            square.setFillColor(matrix[j][i]);
            window.draw(square);
        }
    square.setPosition(pos);
}

using RotationalState = std::vector<sf::Vector2i>;

class Tetromino {
public:
    enum class Type {O, I, T, J, L, S, Z};
    enum class Action {down, right, left, clockwise, anticlockwise};
    Tetromino(Type t): type(t), color(color_map[t]), rots(rotation_map[t]), x(Board::nCol/2), y(0), r(0) {}
    Tetromino &act(Action a);
    std::vector<sf::Vector2i> squares() const;
    Type get_type() const { return type; }
    sf::Color get_color() const { return color; }
private:
    Tetromino &down() { y += 1; return *this; }
    Tetromino &right() { x += 1; return *this; }
    Tetromino &left() { x -= 1; return *this; }
    Tetromino &clockwise() { r = (r + 1) % rots.size(); return *this; }
    Tetromino &anticlockwise() { r = (r - 1) % rots.size(); return *this; }
    static std::map<Type, sf::Color> color_map;
    static std::map<Type, std::vector<RotationalState>> rotation_map;
    Type type;
    sf::Color color;
    std::vector<RotationalState> rots;
    int x;
    int y;
    int r;
};

inline Tetromino &Tetromino::act(Action a) {
    switch (a) {
        case Action::down:
            return down();
        case Action::right:
            return right();
        case Action::left:
            return left();
        case Action::clockwise:
            return clockwise();
        case Action::anticlockwise:
            return anticlockwise();
    }
}

std::vector<sf::Vector2i> Tetromino::squares() const {
    std::vector<sf::Vector2i> positions;
    for (const auto &el : rots[r])
        positions.emplace_back(x + el.x, y + el.y);
    return positions;
}

std::map<Tetromino::Type, sf::Color> Tetromino::color_map = {
    {Type::O, sf::Color::Yellow},
    {Type::I, sf::Color::Cyan},
    {Type::T, sf::Color(127, 2, 122)},
    {Type::J, sf::Color::Blue},
    {Type::L, sf::Color(253, 128, 44)},
    {Type::S, sf::Color::Green},
    {Type::Z, sf::Color::Red}
};

std::map<Tetromino::Type, std::vector<RotationalState>> Tetromino::rotation_map = {
    {Type::O, {
        {{0,0}, {0,1}, {1,0}, {1,1}}
    }},
    {Type::I, {
        {{-1,0}, {0,0}, {1,0}, {2,0}},
        {{0,-1}, {0,0}, {0,1}, {0,2}},
    }},
    {Type::T, {
        {{-1,0}, {0,0}, {1,0}, {0,1}},
        {{-1,0}, {0,0}, {0,-1}, {0,1}},
        {{-1,0}, {0,0}, {1,0}, {0,-1}},
        {{1,0}, {0,0}, {0,-1}, {0,1}},
    }},
    {Type::J, {
        {{-1,0}, {0,0}, {1,0}, {1,1}},
        {{0,-1}, {0,0}, {0,1}, {-1,1}},
        {{-1,0}, {0,0}, {1,0}, {-1,-1}},
        {{0,-1}, {0,0}, {0,1}, {1,-1}}
    }},
    {Type::L, {
        {{-1,0}, {0,0}, {1,0}, {-1,1}},
        {{0,-1}, {0,0}, {0,1}, {-1,-1}},
        {{-1,0}, {0,0}, {1,0}, {1,-1}},
        {{0,-1}, {0,0}, {0,1}, {1,1}}
    }},
    {Type::S, {
        {{-1,1}, {0,1}, {0,0}, {1,0}},
        {{-1,-1}, {-1,0}, {0,0}, {0,1}}
    }},
    {Type::Z, {
        {{-1,0}, {0,0}, {0,1}, {1,1}},
        {{-1,1}, {-1,0}, {0,0}, {0,-1}}
    }},
};

class World {
public:
    World(): u(0, 6), e(static_cast<unsigned>(time(0))), tetromino(Tetromino(random_tetrotype())) {}
    Board board() const;
    World &act_if_legal(Tetromino::Action);
    operator bool() const { return !game_over; }
private:
    std::uniform_int_distribution<unsigned> u;
    std::default_random_engine e;
    Board ground_board;
    Tetromino tetromino;
    bool game_over = false;
    Tetromino::Type random_tetrotype() { return static_cast<Tetromino::Type>(u(e)); }
    World &act_if_legal(Tetromino::Type);
    bool legal() const;
    std::vector<sf::Vector2u> tetsquares() const;
    void remove_filled_lines();
};

inline bool World::legal() const {
    for (const auto &s : tetromino.squares())
        if (ground_board.filled(s.x, s.y))
            return false;
    return true;
}

std::vector<sf::Vector2u> World::tetsquares() const {
    std::vector<sf::Vector2u> sqs;
    for (const auto &v : tetromino.squares())
        sqs.emplace_back(v.x, v.y);
    return sqs;
}

World &World::act_if_legal(Tetromino::Action a) {
    if (game_over)
        return *this;
    World possible_future = *this;
    possible_future.tetromino.act(a);
    if (possible_future.legal())
        tetromino.act(a);
    else if (a == Tetromino::Action::down)
        return act_if_legal(random_tetrotype());
    return *this;
}

World &World::act_if_legal(Tetromino::Type t) {
    ground_board.paint(tetsquares(), tetromino.get_color());
    remove_filled_lines();
    tetromino = Tetromino(t);
    
    if (!legal())
        game_over = true;
    return *this;
}

inline Board World::board() const {
    Board final_board(ground_board);
    final_board.paint(tetsquares(), tetromino.get_color());
    return final_board;
}

void World::remove_filled_lines() {
    for (unsigned y = 0; y < Board::nRow; ++y) {
        bool line_filled = true;
        for (unsigned x = 0; x < Board::nCol; ++x)
            if (!ground_board.filled(x, y))
                line_filled = false;
        if (line_filled)
            ground_board.remove_line(y);
    }
}

class Music {
public:
    explicit Music(std::vector<std::pair<std::string, float>> files) : u(0, files.empty() ? 0 : static_cast<unsigned>(files.size())-1), e(static_cast<unsigned>(time(0))), files(files) {}
    explicit Music(const std::string &folder): Music(get_folder_files(folder)) {}
    void ensure_play();
private:
    std::uniform_int_distribution<unsigned> u;
    std::default_random_engine e;
    const std::vector<std::pair<std::string, float>> files;
    sf::Music music;
    static std::vector<std::pair<std::string, float>> get_folder_files(const std::string &folder);
};

std::vector<std::pair<std::string, float>> Music::get_folder_files(const std::string &folder) {
    std::vector<std::pair<std::string, float>> files;
    float std_volume = 100.0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string file = ent->d_name;
            if (file != "." && file != "..")
                files.emplace_back(std::make_pair(folder + "/" + file, std_volume));
        }
        closedir(dir);
    } else {
        throw std::runtime_error("Folder not found for music loading");
    }
    return files;
}

void Music::ensure_play() {
    if (files.empty()) return;
    if (music.getStatus() == sf::SoundSource::Stopped) {
        auto file = files[u(e)];
        music.openFromFile(file.first);
        music.setVolume(file.second);
        music.play();
    }
}

class Game {
public:
    Game();
    bool window_is_open() const { return window.isOpen(); }
    void process_input();
    void update();
    void render();
    const float frame_duration = 1.0f / 60.0f;
private:
    float sqsize() const { return window.getSize().y / Board::nRow; }
    bool key_pressed(sf::Keyboard::Key k) { return pressed_keys.count(k) ? pressed_keys[k] : false; }
    sf::RenderWindow window;
    sf::RectangleShape square;
    World world;
    std::vector<Tetromino::Action> frame_actions;
    std::map<sf::Keyboard::Key, bool> pressed_keys;
    float standard_turn_duration = 0.6f;
    float quick_turn_duration = 0.1f;
    sf::Clock turn_clock;
    Music music;
};

Game::Game():
window(sf::VideoMode(800, 600), "Tetris"),
square(sf::Vector2f(sqsize(), sqsize())),
pressed_keys {
    {sf::Keyboard::Down, false},
    {sf::Keyboard::Up, false},
    {sf::Keyboard::Space, false}
},
music("Music")
{
    square.setOutlineColor(sf::Color::Black);
    square.setOutlineThickness(1.0f);
    square.setPosition((window.getSize().x - 0.5*window.getSize().y)/2, 0.0f);
}

void Game::process_input() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        if (event.type == sf::Event::KeyPressed && !key_pressed(event.key.code)) {
            switch (event.key.code) {
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                case sf::Keyboard::Right:
                    frame_actions.push_back(Tetromino::Action::right);
                    break;
                case sf::Keyboard::Left:
                    frame_actions.push_back(Tetromino::Action::left);
                    break;
                case sf::Keyboard::Up:
                    frame_actions.push_back(Tetromino::Action::anticlockwise);
                    pressed_keys[event.key.code] = true;
                    break;
                case sf::Keyboard::Down:
                    pressed_keys[event.key.code] = true;
                    break;
                case sf::Keyboard::Space:
                    pressed_keys[event.key.code] = true;
                    break;
                default:
                    break;
            }
        }
            
        if (event.type == sf::Event::KeyReleased)
            pressed_keys[event.key.code] = false;
    }
}

void Game::update() {
    float turn_duration = pressed_keys[sf::Keyboard::Down] || pressed_keys[sf::Keyboard::Space] ? quick_turn_duration : standard_turn_duration;
    
    if (turn_clock.getElapsedTime().asSeconds() > turn_duration) {
        turn_clock.restart();
        frame_actions.push_back(Tetromino::Action::down);
    }
    
    for (const auto &a : frame_actions)
        world.act_if_legal(a);
    frame_actions.clear();
    
    if (!world)
        world = World();
    
    music.ensure_play();
}

void Game::render() {
    window.clear();
    world.board().draw(window, square);
    window.display();
}

int main(int argc, char const** argv)
{
    Game game;
    sf::Clock frame_clock;
    sf::Time elapsed = sf::Time::Zero;
    while (game.window_is_open())
    {
        elapsed += frame_clock.restart();
        if (elapsed.asSeconds() > game.frame_duration) {
            game.process_input();
            game.update();
            game.render();
            elapsed -= sf::seconds(game.frame_duration);
        } else
            sf::sleep(sf::seconds(game.frame_duration) - elapsed);
    }

    return EXIT_SUCCESS;
}
