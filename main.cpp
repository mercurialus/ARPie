#include "ListInterfaces.h"
#include "CapturePacket.h"
#include "ARPieFlooder.h"
#include "ParsePacket.h"

int main(int argc, char* argv[])
{
    const std::string menu("\n===== ARPie Main Menu =====\n"
                                "1. Capture Packets\n"
                                "2. Flood attack someone\n"
                                "3. Exit\n""===========================\n"
                                "Please enter your choice (1-3): ");
    int option=0;
    auto getMenuChoice = [&]()
    {
        while (true) {
            std::cout<<menu;

            if (!(std::cin >> option)) {
                // Handle non-integer input
                std::cin.clear(); // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
                std::cout << "Invalid input. Please enter a number between 1 and 3.\n";
            } else if (option < 1 || option > 3) {
                std::cout << "Invalid choice. Please select a number between 1 and 3.\n";
            } else {
                return option;
            }
        }
    };

    auto Menu =[&]()
    {
        const std::string welcome("Thank you for downloading ARPie! Have fun!");
        std::cout << welcome << "\n\n";

        while (true) {
            const int choice = getMenuChoice();
            if (choice == 1) {
                std::cout << "\nYou selected: Capture Packets\n";
                int opt=0;
                auto interfaceList=listInterface();
                std::cout<<"Please enter the Interface No. for which you want to sniff packets: ";
                std::cin>>opt;
                std::cout<<std::endl;
                const auto userInterface = interfaceList[opt];
                CapturePackets(userInterface);
            } else if (choice == 2) {
                std::cout << "\nYou selected: Flood attack someone\n";

            } else if (choice == 3) {
                std::cout << "\nExiting program. Goodbye!\n";
                break;
            }
        }
    };

    Menu();
}