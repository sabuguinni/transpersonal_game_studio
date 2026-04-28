#include "TranspersonalGameMode.h"
#include "Core/GameFramework/TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set default pawn to our TranspersonalCharacter
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();
    
    // Use default spectator and HUD for now
    // PlayerControllerClass will be set when we have a working controller
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Game started!"));
    InitializeGameSystems();
}

void ATranspersonalGameMode::InitializeGameSystems()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Core systems initialized"));
}
