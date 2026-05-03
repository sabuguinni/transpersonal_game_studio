#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "../Characters/TranspersonalCharacter.h"
#include "../Core/TranspersonalGameMode.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar stats
    TotalActorsInMap = 0;
    CppActorsCount = 0;
    LightingActorsCount = 0;
    bCompilationSuccess = false;
    bMapValidationPassed = false;
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Executar validação automática no início
    ValidateMapState();
    ValidateCppModules();
    GenerateIntegrationReport();
}

void UBuildIntegrationManager::ValidateMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add("World is null - cannot validate map state");
        bMapValidationPassed = false;
        return;
    }
    
    // Limpar dados anteriores
    ValidationErrors.Empty();
    DuplicateLightingActors.Empty();
    
    // Obter todos os actores
    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }
    
    TotalActorsInMap = AllActors.Num();
    
    // Validar lighting
    ValidateLightingActors(World);
    
    // Validar actores C++
    ValidateCppActors(AllActors);
    
    // Validar GameMode
    ValidateGameMode(World);
    
    // Determinar se a validação passou
    bMapValidationPassed = ValidationErrors.Num() == 0;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map validation %s - %d actors, %d errors"), 
           bMapValidationPassed ? TEXT("PASSED") : TEXT("FAILED"), 
           TotalActorsInMap, ValidationErrors.Num());
}

void UBuildIntegrationManager::ValidateLightingActors(UWorld* World)
{
    if (!World) return;
    
    // Contar actores de lighting
    int32 DirectionalLights = 0;
    int32 SkyAtmospheres = 0;
    int32 SkyLights = 0;
    int32 HeightFogs = 0;
    
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        DirectionalLights++;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmospheres++;
        }
        if (Actor->FindComponentByClass<USkyLightComponent>())
        {
            SkyLights++;
        }
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            HeightFogs++;
        }
    }
    
    LightingActorsCount = DirectionalLights + SkyAtmospheres + SkyLights + HeightFogs;
    
    // Detectar duplicados
    if (DirectionalLights > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("DirectionalLight: %d (should be 1)"), DirectionalLights));
        ValidationErrors.Add(FString::Printf(TEXT("Too many DirectionalLights: %d"), DirectionalLights));
    }
    if (SkyAtmospheres > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("SkyAtmosphere: %d (should be 1)"), SkyAtmospheres));
        ValidationErrors.Add(FString::Printf(TEXT("Too many SkyAtmospheres: %d"), SkyAtmospheres));
    }
    if (SkyLights > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("SkyLight: %d (should be 1)"), SkyLights));
        ValidationErrors.Add(FString::Printf(TEXT("Too many SkyLights: %d"), SkyLights));
    }
    if (HeightFogs > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("ExponentialHeightFog: %d (should be 1)"), HeightFogs));
        ValidationErrors.Add(FString::Printf(TEXT("Too many HeightFogs: %d"), HeightFogs));
    }
}

void UBuildIntegrationManager::ValidateCppActors(const TArray<AActor*>& AllActors)
{
    CppActorsCount = 0;
    
    // Contar actores C++ específicos do projecto
    int32 TranspersonalCharacters = 0;
    int32 PlayerStarts = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        // Verificar se é uma classe C++ do nosso projecto
        if (ClassName.Contains("Transpersonal") || ClassName.Contains("PCG") || 
            ClassName.Contains("Foliage") || ClassName.Contains("Crowd"))
        {
            CppActorsCount++;
        }
        
        // Contar tipos específicos
        if (Actor->IsA<ATranspersonalCharacter>())
        {
            TranspersonalCharacters++;
        }
        if (Actor->IsA<APlayerStart>())
        {
            PlayerStarts++;
        }
    }
    
    // Validar contagens esperadas
    if (PlayerStarts == 0)
    {
        ValidationErrors.Add("No PlayerStart found in map");
    }
    else if (PlayerStarts > 1)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Too many PlayerStarts: %d"), PlayerStarts));
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Found %d C++ actors, %d TranspersonalCharacters, %d PlayerStarts"), 
           CppActorsCount, TranspersonalCharacters, PlayerStarts);
}

void UBuildIntegrationManager::ValidateGameMode(UWorld* World)
{
    if (!World) return;
    
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        ValidationErrors.Add("No GameMode set for the world");
        return;
    }
    
    // Verificar se é o nosso GameMode
    if (GameMode->IsA<ATranspersonalGameMode>())
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: TranspersonalGameMode is active"));
    }
    else
    {
        ValidationErrors.Add(FString::Printf(TEXT("Wrong GameMode: %s (expected TranspersonalGameMode)"), 
                                           *GameMode->GetClass()->GetName()));
    }
}

void UBuildIntegrationManager::ValidateCppModules()
{
    // Limpar listas anteriores
    LoadedCppClasses.Empty();
    FailedCppClasses.Empty();
    
    // Lista de classes críticas para validar
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
    
    int32 LoadedCount = 0;
    
    for (const FString& ClassPath : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedCppClasses.Add(ClassPath);
            LoadedCount++;
        }
        else
        {
            FailedCppClasses.Add(ClassPath);
        }
    }
    
    bCompilationSuccess = (FailedCppClasses.Num() == 0);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: C++ Module validation - %d/%d classes loaded"), 
           LoadedCount, CriticalClasses.Num());
}

void UBuildIntegrationManager::CleanupDuplicateLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Limpar DirectionalLights duplicados (manter apenas o primeiro)
    TArray<ADirectionalLight*> DirectionalLights;
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        DirectionalLights.Add(*ActorItr);
    }
    
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        if (DirectionalLights[i])
        {
            DirectionalLights[i]->Destroy();
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Destroyed duplicate DirectionalLight"));
        }
    }
    
    // Limpar outros duplicados de lighting seria similar...
    // Por agora, apenas log para não quebrar o mapa
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Lighting cleanup completed"));
}

void UBuildIntegrationManager::GenerateIntegrationReport()
{
    FString Report = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorsInMap);
    Report += FString::Printf(TEXT("C++ Actors: %d\n"), CppActorsCount);
    Report += FString::Printf(TEXT("Lighting Actors: %d\n"), LightingActorsCount);
    Report += FString::Printf(TEXT("Compilation Success: %s\n"), bCompilationSuccess ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Map Validation: %s\n"), bMapValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    Report += TEXT("\nLoaded C++ Classes:\n");
    for (const FString& ClassName : LoadedCppClasses)
    {
        Report += FString::Printf(TEXT("  ✓ %s\n"), *ClassName);
    }
    
    Report += TEXT("\nFailed C++ Classes:\n");
    for (const FString& ClassName : FailedCppClasses)
    {
        Report += FString::Printf(TEXT("  ✗ %s\n"), *ClassName);
    }
    
    Report += TEXT("\nValidation Errors:\n");
    for (const FString& Error : ValidationErrors)
    {
        Report += FString::Printf(TEXT("  ! %s\n"), *Error);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

bool UBuildIntegrationManager::IsIntegrationHealthy() const
{
    return bCompilationSuccess && bMapValidationPassed && ValidationErrors.Num() == 0;
}

FString UBuildIntegrationManager::GetIntegrationStatus() const
{
    if (IsIntegrationHealthy())
    {
        return TEXT("HEALTHY");
    }
    else if (!bCompilationSuccess)
    {
        return TEXT("COMPILATION_FAILED");
    }
    else if (!bMapValidationPassed)
    {
        return TEXT("MAP_VALIDATION_FAILED");
    }
    else
    {
        return TEXT("ERRORS_DETECTED");
    }
}