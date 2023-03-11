#include <SFML/Config.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>

unsigned char *dt, *dt0;
double de = 1.0, ad = 0.000000000001, kk = 0.054;
double xx = 0.0, yy = 0.0, xm = 0.0, ym = 0.0;
const int xa = 842, ya = 620;

double lerp( double a, double b, double t )
{ return  a * ( 1 - sin(t) ) + b * cos(t); }

struct thP { int y1, y2; };

struct Cmplex { double ma, mb;

    void square()
    {   double tmp = ( ma * ma ) - ( mb * mb);
        mb = 2.0 * ma * mb;
        ma = tmp;   }

    double magnitude()
    {   return sqrt( ( ma * mb ) + ( ma * mb ) );   }

    void add(Cmplex *c0)
    {   ma += c0->ma; mb += c0->mb;   }

};

struct Vec3d { double x,y,z;};

void blur( thP pd )
{

  for ( int x = 1; x < xa; x++ ) { for ( int y = pd.y1; y <= pd.y2; y++ ) {

                int p = ( ( y * xa ) + x ) * 4, pt;

                double r0 = static_cast<double>(dt[p+0]), res{};

                pt = ( ( ( y - 0 ) * xa ) + ( x - 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y - 1 ) * xa ) + ( x - 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y + 1 ) * xa ) + ( x - 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y - 1 ) * xa ) + ( x - 0 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y + 1 ) * xa ) + ( x - 0 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y + 0 ) * xa ) + ( x + 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y - 1 ) * xa ) + ( x + 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);
                pt = ( ( ( y + 1 ) * xa ) + ( x + 1 ) ) * 4;

                res += static_cast<double>(dt[pt+0]);

                r0 *= 0.876; res *= 0.123;
                unsigned char red = r0 + res; if ( red > 148 ) red = 148;

                dt0[p+0] = red; dt0[p+1] = dt[p+1]; dt0[p+2] = dt[p+2]; dt0[p+3] = 255;

                } }
}

const int xc = xa / 2, yc = ya / 2;
void outPut( thP pd ) {

    for ( int x = 0; x <= xa; x++ ) { for ( int y = pd.y1; y <= pd.y2; y++) {

        int p = ( ( xa * y ) + x ) * 4;

        double xq = (double)x  / de, yq = (double)y / de,
               a = ( xq + xm + xx - ( ( xa / de ) / 2 ) ) / ( xa / 4.0 ),
               b = ( yq + ym + yy - ( ( ya / de ) / 2 ) ) / ( ya / 4.0 );

        struct Cmplex c, z;
        c.ma = a; c.mb = b;
        z.ma = 0.0; z.mb = 0.0;

        unsigned char i = 0; double m;

        while ( i < 69 ) { i++; z.square(); z.add(&c); m = z.magnitude(); if ( m > 10.0 ) break; }

       if ( i < 69 ) { m *= 0.01;

           double
                   r = (unsigned char) i + tan( m * z.ma / b ) / ( ( sin ( z.mb ) ) ),
                   g = (unsigned char)(double)( !i | i * 16 ) * acos ( m * 0.39 ) ,
                   b = (unsigned char)((double)i * 12 ) * atan ( z.mb * z.ma / ( 0.00001 * m ) ) ;

        dt[p+0] = r;
        dt[p+1] = g;
        dt[p+2] = b;

        } else { dt[p] = 0; dt[p+1] = 0; dt[p+2] = 0; }

    } }
}

class Timer
{
private:
    using clock_type = std::chrono::steady_clock;
    using second_type = std::chrono::duration< double, std::ratio<1,60> >;
    std::chrono::time_point<clock_type> m_beg;

public:
    Timer() : m_beg { clock_type::now() } { }

    void reset() { m_beg = clock_type::now(); }

    double elapsed() const
    { return std::chrono::duration_cast<second_type>( clock_type::now() - m_beg ).count(); }
};

int main()
{
    sf::RenderWindow w0(sf::VideoMode::getDesktopMode(), "T"); w0.setVisible(false);

    sf::Vector2i curpos; sf::Vector2u wsize = w0.getSize();

    int x0 = ( wsize.x / 2 ) - xa / 2, y0 = ( wsize.y / 2) - ya / 2;

    sf::RenderWindow w(sf::VideoMode(xa,ya),"Mandelbrot Set",sf::Style::Close);

    w.setPosition( {x0, y0} ); w.setSize( {xa, ya} );


    sf::Texture img; if ( !img.create(xa, ya) ) std::cout << "Img error!" << std::endl;

    sf::Sprite spr; spr.setTextureRect(sf::IntRect(0, 0, xa, ya));

    spr.setTexture(img); spr.setPosition(-1,0);


    dt = new unsigned char [xa*(ya+2)*4]{}; dt0 = new unsigned char [xa*(ya+2)*4]{}; Timer t;
    for ( int n = 0; n <= xa*ya*4; n += 4 ) { dt0[n+3] = 255; dt[n+3] = 255; }

    int bpress = 1, isout = 0, isin = 0, num_threads = std::thread::hardware_concurrency();

    num_threads = ( num_threads >= 12 ) ? 12 : num_threads;

    double px = 0.0, py = 0.0, ppx = 0.0, ppy = 0.0, waittime = 3.9 * ( num_threads / 12 );

    std::thread *th; std::vector<std::thread*> tcoll;
    struct thP p[num_threads];

    int nl = ( ya / num_threads ) - 1;
    unsigned char re = static_cast<int>(ya) % num_threads;

    int z = 0; for ( int i = 0; i < num_threads; i++ ) {

        p[i].y1 = z; z += nl; if ( re != 0 ) { z++; re--; }

        if ( i == num_threads - 1 ) p[i].y2 = ya; else p[i].y2 = z; }


    while ( w.isOpen() )  { sf::Event event; if ( t.elapsed() > waittime ) { t.reset();

        std::cout << "." << std::endl;

        if ( bpress ) {

          for ( int i = 0; i < num_threads; i++ )
          { th = new std::thread { outPut, p[i] }; tcoll.push_back(th); }

          for ( int i = 0; i < num_threads; i++ )
          { tcoll[i]->join(); delete tcoll[i]; }

          tcoll.clear();

          for ( int i = 0; i < num_threads; i++ )
          { th = new std::thread { blur, p[i] }; tcoll.push_back(th); }

          for ( int i = 0; i < num_threads; i++ )
          { tcoll[i]->join(); delete tcoll[i]; }

          tcoll.clear(); bpress = 0;

        }

        img.update(dt0); w.draw(spr); w.display();

        if ( isin )  { ad -= kk; de = exp(ad); kk += 0.000034; bpress = 1; }
        if ( isout ) { ad += kk; de = exp(ad); kk -= 0.000034; bpress = 1; }

        while ( w.pollEvent(event) )
        {
            if ( event.type == sf::Event::Closed ) w.close();

            if ( event.type == sf::Event::KeyReleased ) switch ( event.key.code ) {

                case sf::Keyboard::A : printf("A\n"); if ( !isin ) { isout =! isout; bpress = 1; } break;

                case sf::Keyboard::Z : printf("Z\n"); if ( !isout ) { isin =! isin; bpress = 1; } break;

                case sf::Keyboard::Space : printf("SPACE\n"); break;

                case sf::Keyboard::Escape : w.close(); break; }

            if ( event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left ) {

                curpos =  sf::Mouse::getPosition(w);

                xx = ( xa / 2 - curpos.x ) / de; yy = ( ya / 2 - curpos.y ) / de;
                xx -= ( xx * 2 ) - px; yy -= ( yy * 2 ) - py; px = xx; py = yy;

                bpress = 1; break; }

        } }
    }

    delete[] dt; delete[] dt0;

    return 0;
}
