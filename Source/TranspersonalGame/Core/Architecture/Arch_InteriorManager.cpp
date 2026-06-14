#include "Arch_InteriorManager.h"
#include "Engine/World.h"
#include "Engine/TriggerVolume.h"
#include "Components/BrushComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UArch_InteriorManager::UArch_InteriorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5 seconds

    bPlayerInsideInterior = false;
    InteriorCheckRadius = 500.0f;
    TemperatureModifier = 1.0f;
    HumidityModifier = 1.0f;
}

void UArch_InteriorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default exterior conditions
    CurrentInterior = FArch_InteriorData();
    CurrentInterior.InteriorName = TEXT("Exterior");
    CurrentInterior.Temperature = 15.0f; // Base outdoor temperature
    CurrentInterior.Humidity = 60.0f;    // Base outdoor humidity
    CurrentInterior.WindProtection = 0.0f; // No wind protection outside
}

void UArch_InteriorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CheckPlayerInteriorStatus();
}

void UArch_InteriorManager::RegisterInterior(const FArch_InteriorData& InteriorData, ATriggerVolume* TriggerVolume)
{
    if (!TriggerVolume)
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: Cannot register interior without trigger volume"));
        return;
    }

    // Check if interior already exists
    FArch_InteriorData* ExistingInterior = FindInteriorByName(InteriorData.InteriorName);
    if (ExistingInterior)
    {
        *ExistingInterior = InteriorData;
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Updated existing interior: %s"), *InteriorData.InteriorName);
    }
    else
    {
        RegisteredInteriors.Add(InteriorData);
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Registered new interior: %s"), *InteriorData.InteriorName);
    }
}

void UArch_InteriorManager::UnregisterInterior(const FString& InteriorName)
{
    for (int32 i = RegisteredInteriors.Num() - 1; i >= 0; i--)
    {
        if (RegisteredInteriors[i].InteriorName == InteriorName)
        {
            RegisteredInteriors.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Unregistered interior: %s"), *InteriorName);
            break;
        }
    }
}

bool UArch_InteriorManager::IsPlayerInsideInterior() const
{
    return bPlayerInsideInterior;
}

FArch_InteriorData UArch_InteriorManager::GetCurrentInterior() const
{
    return CurrentInterior;
}

void UArch_InteriorManager::OnPlayerEnterInterior(const FArch_InteriorData& InteriorData)
{
    if (!bPlayerInsideInterior || CurrentInterior.InteriorName != InteriorData.InteriorName)
    {
        bPlayerInsideInterior = true;
        CurrentInterior = InteriorData;
        
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Player entered interior: %s"), *InteriorData.InteriorName);
        UE_LOG(LogTemp, Log, TEXT("  Temperature: %.1f°C, Humidity: %.1f%%, Wind Protection: %.1f%%"), 
               InteriorData.Temperature, InteriorData.Humidity, InteriorData.WindProtection * 100.0f);
    }
}

void UArch_InteriorManager::OnPlayerExitInterior()
{
    if (bPlayerInsideInterior)
    {
        bPlayerInsideInterior = false;
        
        // Reset to exterior conditions
        CurrentInterior = FArch_InteriorData();
        CurrentInterior.InteriorName = TEXT("Exterior");
        CurrentInterior.Temperature = 15.0f;
        CurrentInterior.Humidity = 60.0f;
        CurrentInterior.WindProtection = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Player exited to exterior"));
    }
}

float UArch_InteriorManager::GetInteriorTemperature() const
{
    return CurrentInterior.Temperature * TemperatureModifier;
}

float UArch_InteriorManager::GetInteriorHumidity() const
{
    return CurrentInterior.Humidity * HumidityModifier;
}

float UArch_InteriorManager::GetWindProtectionLevel() const
{
    return CurrentInterior.WindProtection;
}

bool UArch_InteriorManager::HasFirePit() const
{
    return CurrentInterior.bHasFirePit;
}

bool UArch_InteriorManager::HasSleepingArea() const
{
    return CurrentInterior.bHasSleepingArea;
}

bool UArch_InteriorManager::HasStorageArea() const
{
    return CurrentInterior.bHasStorageArea;
}

void UArch_InteriorManager::CreateTestInterior()
{
    FArch_InteriorData TestInterior;
    TestInterior.InteriorName = TEXT("Test Cave");
    TestInterior.BiomeType = EBiomeType::Forest;
    TestInterior.Temperature = 18.0f;
    TestInterior.Humidity = 45.0f;
    TestInterior.WindProtection = 0.9f;
    TestInterior.bHasFirePit = true;
    TestInterior.bHasSleepingArea = true;
    TestInterior.bHasStorageArea = false;
    
    RegisteredInteriors.Add(TestInterior);
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Created test interior"));
}

void UArch_InteriorManager::ClearAllInteriors()
{
    RegisteredInteriors.Empty();
    OnPlayerExitInterior(); // Reset to exterior
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Cleared all registered interiors"));
}

void UArch_InteriorManager::CheckPlayerInteriorStatus()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    bool bFoundInterior = false;

    // Check all registered interiors for player presence
    for (const FArch_InteriorData& Interior : RegisteredInteriors)
    {
        // For now, use simple distance check
        // In a full implementation, you'd check trigger volume overlaps
        // This is a placeholder for the trigger volume system
        
        // Simulate interior detection based on naming convention
        if (Interior.InteriorName.Contains(TEXT("Cave")) && 
            FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per check for demo
        {
            OnPlayerEnterInterior(Interior);
            bFoundInterior = true;
            break;
        }
    }

    if (!bFoundInterior && bPlayerInsideInterior)
    {
        OnPlayerExitInterior();
    }
}

FArch_InteriorData* UArch_InteriorManager::FindInteriorByName(const FString& InteriorName)
{
    for (FArch_InteriorData& Interior : RegisteredInteriors)
    {
        if (Interior.InteriorName == InteriorName)
        {
            return &Interior;
        }
    }
    return nullptr;
}