
#include "../crypto_camellia.hh"
#include "crypto_file.hh"
#include "../crypto_util.hh"
#include "../file_util.hh"
#include <unistd.h>

namespace {
const std::string PLAIN = "Abenddämmerung\n"
"\n"
"Sieh, des Verbrechers Freund, der holde Abend, naht\n"
"Mit leisem Raubtierschritt, der Helfer bei der Tat;\n"
"Der Himmel schliesst nun sacht des schweren Vorhangs Falten,\n"
"Zu Tieren wandeln sich die menschlichen Gestalten.\n"
"\n"
"O Abend lieb und hold, wie heiss wirst du ersehnt\n"
"Von einem, der mit Lust die müden Arme dehnt\n"
"Und ohne Lügen spricht: Der Tag war voller Lasten! –\n"
"Du bist's, der Schmerzen stillt und Ruhe gibt und Rasten\n"
"Dem Denker, der voll Trotz die müde Stirne hält\n"
"Dem Arbeitsmann, der dumpf hin auf sein Lager fällt.\n"
"\n"
"Indes erhebt sich schwer der bösen Geister Meute,\n"
"Sie flattern durch die Luft wie vielgeschäftige Leute,\n"
"Sie poltern an die Tür, sie stossen an das Dach.\n"
"Und wo ein Lichtschein wird im Windstoss flackernd wach,\n"
"Da lebt die Unzucht auf in dumpfer Gassen Enge;\n"
"Gleich dem Ameisenhauf öffnet sie Gäng' um Gänge;\n"
"Sie bahnt geheimen Weg allüberall und gleicht\n"
"Dem Feind im Hinterhalt, der tückisch uns umschleicht;\n"
"\n"
"Im Schoss der Stadt rührt sie den Unrat, der sie mehrt,\n"
"Ein Wurm, der von der Kraft des Menschen lebt und zehrt.\n"
"Jetzt hört man's da und dort in Küchen leise zischen,\n"
"Theater kreischen auf, Orchester brummt dazwischen;\n"
"Die Säle, drin das Spiel Rausch gibt den schlaffen Hirnen,\n"
"Sie füllen sich nun rasch mit Gaunern und mit Dirnen;\n"
"Die Diebe, denen nie das Handwerk Ruhe lässt,\n"
"Beginnen ihr Geschäft, bezwingen sanft und fest\n"
"Die Türen und den Schrein um ein paar Tage Leben\n"
"Und, um der Freundin Gold und seidnen Tand zu geben.\n"
"\n"
"Jetzt sammle dich, mein Sinn, und richte dich empor,\n"
"In diesem Augenblick verschliess dem Lärm dein Ohr.\n"
"Die Stunde ist's, da Gram und Schmerzen sich verschlimmern,\n"
"Da uns die finstre Nacht die Kehle würgt, und Wimmern\n"
"Die Hospitale füllt, da still der Kranken Heer\n"
"Zum grossen Abgrund wallt. – Ja, mancher kommt nie mehr\n"
"Und isst die Suppe still und träumt und blickt ins Feuer\n"
"Ganz nah beim Herd und nah der Seele, die ihm teuer.\n"
"\n"
"Und viele kannten nie die Süssigkeit, die schwebt\n"
"Um einen Platz am Herd, und haben nie gelebt!\n"
"\n"
"Charles Baudelaire";

const auto CIPHER = "crypto_test_input.crypt";

void print_usage(int, char *argv[])
{
    std::cout << "USAGE:\n" << std::string(argv[0])
              << " password\n";
}

}

// create ciphertext in programm, then decrypt again works.
// rerad in encrypted file fails
bool test1(const std::string &pw)
{
/* Works
    const password_from_stdin pw2(6);
    std::cout << "password_from_stdin: ";
    if(pw2.have_password)
        std:: cout << "\"" << pw2.password << "\"\n";
    else {
        std::cout << "failed.\n";
        return false;
    }
*/

    using namespace libaan;
    crypto_file file_io(CIPHER);
    crypto_file::error_type err = crypto_file::NO_ERROR;
    err = file_io.read(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::read() failed: "
                  << crypto_file::error_string(err) << "\n";
        return false;
    }

    // set content
    file_io.get_decrypted_buffer().assign(PLAIN);

    err = file_io.write(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::write failed: "
                  << crypto_file::error_string(err) << "\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "encryption and write successfull.\nReading encrypted file again..\n";

    crypto_file file_io2(CIPHER);
    err = file_io2.read(pw);
    if(err != crypto_file::NO_ERROR) {
        std::cout << "crypto_file::read() failed: "
                  << crypto_file::error_string(err) << "\n";
        return false;
    }
    if(file_io2.get_decrypted_buffer() != PLAIN) {
        std::cout << "Error: plain != Decrypt(Encrypt(plain))\n";
        return false;
    }
    std::cout << "Success: plain == Decrypt(Encrypt(plain))\n";

    return true;
}

int main(int argc, char *argv[])
{
    print_usage(argc, argv);
    if(argc != 2) {
        exit(EXIT_FAILURE);
    }

    const std::string pw(argv[1]);
    test1(pw); 

    exit(EXIT_SUCCESS);
}
