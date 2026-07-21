#include "Eng_SystemValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"

void UEng_SystemValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Initialized"));
    
    LastValidationResults.Empty();
    RegisteredSystems.Empty();
    TotalSystemsValidated = 0;
}

void UEng_SystemValidator::Deinitialize()
{
    RegisteredSystems.Empty();
    LastValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Deinitialized"));
    Super::Deinitialize();
}

TArray<FEng_ValidationResult> UEng_SystemValidator::ValidateAllSystems()
{
    float StartTime = FPlatformTime::Seconds();
    LastValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Starting full system validation"));
    
    // Validate core systems
    LastValidationResults.Add(ValidateGameMode());
    LastValidationResults.Add(ValidatePlayerController());
    LastValidationResults.Add(ValidateCharacter());
    LastValidationResults.Add(ValidateWorldSystems());
    LastValidationResults.Add(ValidatePerformance());
    
    // Validate registered custom systems
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_ValidationResult Result = ValidateSystem(SystemPair.Key);
        LastValidationResults.Add(Result);
    }
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    TotalSystemsValidated = LastValidationResults.Num();
    
    // Log summary
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;
    for (const auto& Result : LastValidationResults)
    {
        if (Result.bIsValid) ValidSystems++;
        else InvalidSystems++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Validation complete - %d valid, %d invalid (%.3fs)"), 
           ValidSystems, InvalidSystems, LastValidationTime);
    
    return LastValidationResults;
}

FEng_ValidationResult UEng_SystemValidator::ValidateSystem(const FString& SystemName)
{
    FEng_ValidationResult Result;
    Result.SystemName = SystemName;
    Result.ValidationTime = FPlatformTime::Seconds();
    
    // Check if system is registered
    if (RegisteredSystems.Contains(SystemName))
    {
        TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[SystemName];
        if (SystemPtr.IsValid())
        {
            Result.bIsValid = true;
            Result.ErrorMessage = TEXT("System is valid and accessible");
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("System object is no longer valid (garbage collected)");
            Result.ErrorCount = 1;
        }
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorMessage = FString::Printf(TEXT("System '%s' is not registered"), *SystemName);
        Result.ErrorCount = 1;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    LogValidationResult(Result);
    
    return Result;
}

bool UEng_SystemValidator::IsSystemValid(const FString& SystemName)
{
    FEng_ValidationResult Result = ValidateSystem(SystemName);
    return Result.bIsValid;
}

void UEng_SystemValidator::RunFullValidation()
{
    TArray<FEng_ValidationResult> Results = ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FULL SYSTEM VALIDATION REPORT ==="));
    
    for (const auto& Result : Results)
    {
        FString Status = Result.bIsValid ? TEXT("PASS") : TEXT("FAIL");
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s - %s (%.3fs)"), 
               *Status, *Result.SystemName, *Result.ErrorMessage, Result.ValidationTime);
        
        if (Result.ErrorCount > 0 || Result.WarningCount > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("  Errors: %d, Warnings: %d"), 
                   Result.ErrorCount, Result.WarningCount);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE ==="));
}

void UEng_SystemValidator::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (SystemObject)
    {
        RegisteredSystems.Add(SystemName, SystemObject);
        UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Registered system '%s'"), *SystemName);
    }
}

void UEng_SystemValidator::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemValidator: Unregistered system '%s'"), *SystemName);
    }
}

FEng_ValidationResult UEng_SystemValidator::ValidateGameMode()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("GameMode");
    Result.ValidationTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("World is null");
        Result.ErrorCount = 1;
    }
    else
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (GameMode)
        {
            Result.bIsValid = true;
            Result.ErrorMessage = FString::Printf(TEXT("GameMode active: %s"), 
                                                 *GameMode->GetClass()->GetName());
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("No GameMode found");
            Result.ErrorCount = 1;
        }
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

FEng_ValidationResult UEng_SystemValidator::ValidatePlayerController()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("PlayerController");
    Result.ValidationTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            Result.bIsValid = true;
            Result.ErrorMessage = FString::Printf(TEXT("PlayerController active: %s"), 
                                                 *PC->GetClass()->GetName());
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("No PlayerController found");
            Result.ErrorCount = 1;
        }
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("World is null");
        Result.ErrorCount = 1;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

FEng_ValidationResult UEng_SystemValidator::ValidateCharacter()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("Character");
    Result.ValidationTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
            if (Character)
            {
                Result.bIsValid = true;
                Result.ErrorMessage = FString::Printf(TEXT("Character active: %s"), 
                                                     *Character->GetClass()->GetName());
            }
            else
            {
                Result.bIsValid = false;
                Result.ErrorMessage = TEXT("Pawn is not a Character");
                Result.ErrorCount = 1;
            }
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("No possessed pawn found");
            Result.WarningCount = 1; // Warning, not error - might be in menu
        }
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("World is null");
        Result.ErrorCount = 1;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

FEng_ValidationResult UEng_SystemValidator::ValidateWorldSystems()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("WorldSystems");
    Result.ValidationTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        if (AllActors.Num() > 0)
        {
            Result.bIsValid = true;
            Result.ErrorMessage = FString::Printf(TEXT("World contains %d actors"), AllActors.Num());
            
            if (AllActors.Num() > 8000)
            {
                Result.WarningCount = 1;
                Result.ErrorMessage += TEXT(" (high actor count - performance warning)");
            }
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("World contains no actors");
            Result.ErrorCount = 1;
        }
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("World is null");
        Result.ErrorCount = 1;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

FEng_ValidationResult UEng_SystemValidator::ValidatePerformance()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("Performance");
    Result.ValidationTime = FPlatformTime::Seconds();
    
    // Simple performance check
    float FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    if (FrameTime < 33.33f) // 30 FPS threshold
    {
        Result.bIsValid = true;
        Result.ErrorMessage = FString::Printf(TEXT("Performance OK (%.2f ms frame time)"), FrameTime);
    }
    else
    {
        Result.bIsValid = false;
        Result.ErrorMessage = FString::Printf(TEXT("Performance critical (%.2f ms frame time)"), FrameTime);
        Result.WarningCount = 1;
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - Result.ValidationTime;
    return Result;
}

bool UEng_SystemValidator::ValidateObjectExists(UObject* Object, const FString& ObjectName, FString& OutError)
{
    if (!Object)
    {
        OutError = FString::Printf(TEXT("%s is null"), *ObjectName);
        return false;
    }
    
    if (!IsValid(Object))
    {
        OutError = FString::Printf(TEXT("%s is not valid"), *ObjectName);
        return false;
    }
    
    return true;
}

bool UEng_SystemValidator::ValidateComponentSetup(AActor* Actor, const FString& ActorName, FString& OutError)
{
    if (!ValidateObjectExists(Actor, ActorName, OutError))
    {
        return false;
    }
    
    TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
    if (Components.Num() == 0)
    {
        OutError = FString::Printf(TEXT("%s has no components"), *ActorName);
        return false;
    }
    
    return true;
}

void UEng_SystemValidator::LogValidationResult(const FEng_ValidationResult& Result)
{
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("SystemValidator: [PASS] %s - %s"), 
               *Result.SystemName, *Result.ErrorMessage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemValidator: [FAIL] %s - %s"), 
               *Result.SystemName, *Result.ErrorMessage);
    }
}