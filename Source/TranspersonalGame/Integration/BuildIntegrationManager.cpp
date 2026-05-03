#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar contadores
    TotalActorsInMap = 0;
    CppActorsCount = 0;
    LightingActorsCount = 0;
    bCompilationSuccess = false;
    bMapValidationPassed = false;
    
    // Inicializar arrays
    LoadedCppClasses.Empty();
    FailedCppClasses.Empty();
    DuplicateLightingActors.Empty();
    ValidationErrors.Empty();
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager iniciado"));
    
    // Executar validação inicial
    ValidateMapState();
    ValidateCppModules();
    GenerateIntegrationReport();
}

void UBuildIntegrationManager::ValidateMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Mundo não encontrado"));
        ValidationErrors.Add(TEXT("World not found"));
        return;
    }
    
    // Contar todos os actores
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorsInMap = AllActors.Num();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Total de actores no mapa: %d"), TotalActorsInMap);
    
    // Verificar actores de lighting
    ValidateLightingActors(World);
    
    // Verificar actores C++
    ValidateCppActors(AllActors);
    
    // Verificar GameMode
    ValidateGameMode(World);
    
    bMapValidationPassed = ValidationErrors.Num() == 0;
}

void UBuildIntegrationManager::ValidateLightingActors(UWorld* World)
{
    if (!World) return;
    
    // Verificar DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("DirectionalLight: %d instances"), DirectionalLights.Num()));
        UE_LOG(LogBuildIntegration, Warning, TEXT("Múltiplos DirectionalLights encontrados: %d"), DirectionalLights.Num());
    }
    else if (DirectionalLights.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No DirectionalLight found in map"));
        UE_LOG(LogBuildIntegration, Error, TEXT("Nenhum DirectionalLight encontrado"));
    }
    
    LightingActorsCount = DirectionalLights.Num();
    
    // Verificar SkyLight
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 1)
    {
        DuplicateLightingActors.Add(FString::Printf(TEXT("SkyLight: %d instances"), SkyLights.Num()));
    }
    
    LightingActorsCount += SkyLights.Num();
}

void UBuildIntegrationManager::ValidateCppActors(const TArray<AActor*>& AllActors)
{
    CppActorsCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        // Verificar se é uma classe C++ do nosso módulo
        if (ClassName.Contains(TEXT("Transpersonal")) || 
            ClassName.Contains(TEXT("PCG")) || 
            ClassName.Contains(TEXT("Foliage")) || 
            ClassName.Contains(TEXT("Crowd")))
        {
            CppActorsCount++;
            UE_LOG(LogBuildIntegration, Log, TEXT("Actor C++ encontrado: %s (%s)"), 
                   *Actor->GetName(), *ClassName);
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Total de actores C++ no mapa: %d"), CppActorsCount);
}

void UBuildIntegrationManager::ValidateGameMode(UWorld* World)
{
    if (!World) return;
    
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (!GameMode)
    {
        ValidationErrors.Add(TEXT("No GameMode found"));
        UE_LOG(LogBuildIntegration, Error, TEXT("Nenhum GameMode encontrado"));
        return;
    }
    
    FString GameModeClass = GameMode->GetClass()->GetName();
    UE_LOG(LogBuildIntegration, Log, TEXT("GameMode actual: %s"), *GameModeClass);
    
    if (!GameModeClass.Contains(TEXT("Transpersonal")))
    {
        ValidationErrors.Add(FString::Printf(TEXT("GameMode is not TranspersonalGameMode: %s"), *GameModeClass));
    }
}

void UBuildIntegrationManager::ValidateCppModules()
{
    // Lista de classes C++ que devem estar carregadas
    TArray<FString> ExpectedClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
    
    LoadedCppClasses.Empty();
    FailedCppClasses.Empty();
    
    for (const FString& ClassName : ExpectedClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedCppClasses.Add(ClassName);
            UE_LOG(LogBuildIntegration, Log, TEXT("Classe C++ carregada: %s"), *ClassName);
        }
        else
        {
            FailedCppClasses.Add(ClassName);
            UE_LOG(LogBuildIntegration, Error, TEXT("Falha ao carregar classe C++: %s"), *ClassName);
        }
    }
    
    bCompilationSuccess = FailedCppClasses.Num() == 0;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Classes carregadas: %d, Falharam: %d"), 
           LoadedCppClasses.Num(), FailedCppClasses.Num());
}

void UBuildIntegrationManager::GenerateIntegrationReport()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== RELATÓRIO DE INTEGRAÇÃO ==="));
    UE_LOG(LogBuildIntegration, Log, TEXT("Total de actores: %d"), TotalActorsInMap);
    UE_LOG(LogBuildIntegration, Log, TEXT("Actores C++: %d"), CppActorsCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("Actores de lighting: %d"), LightingActorsCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("Compilação bem-sucedida: %s"), bCompilationSuccess ? TEXT("SIM") : TEXT("NÃO"));
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação do mapa: %s"), bMapValidationPassed ? TEXT("PASSOU") : TEXT("FALHOU"));
    
    if (DuplicateLightingActors.Num() > 0)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Duplicados de lighting encontrados:"));
        for (const FString& Duplicate : DuplicateLightingActors)
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("  %s"), *Duplicate);
        }
    }
    
    if (ValidationErrors.Num() > 0)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Erros de validação encontrados:"));
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("  %s"), *Error);
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("=== FIM DO RELATÓRIO ==="));
}

bool UBuildIntegrationManager::IsIntegrationHealthy() const
{
    return bCompilationSuccess && bMapValidationPassed && ValidationErrors.Num() == 0;
}

void UBuildIntegrationManager::CleanupDuplicateLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Iniciando limpeza de duplicados de lighting"));
    
    // Limpar DirectionalLights duplicados
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 1)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Removendo %d DirectionalLights duplicados"), DirectionalLights.Num() - 1);
        
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            if (DirectionalLights[i])
            {
                DirectionalLights[i]->Destroy();
            }
        }
    }
    
    // Limpar SkyLights duplicados
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 1)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Removendo %d SkyLights duplicados"), SkyLights.Num() - 1);
        
        for (int32 i = 1; i < SkyLights.Num(); i++)
        {
            if (SkyLights[i])
            {
                SkyLights[i]->Destroy();
            }
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Limpeza de duplicados concluída"));
}

FString UBuildIntegrationManager::GetIntegrationStatus() const
{
    if (IsIntegrationHealthy())
    {
        return TEXT("HEALTHY");
    }
    else if (bCompilationSuccess && !bMapValidationPassed)
    {
        return TEXT("COMPILATION_OK_MAP_ISSUES");
    }
    else if (!bCompilationSuccess && bMapValidationPassed)
    {
        return TEXT("MAP_OK_COMPILATION_ISSUES");
    }
    else
    {
        return TEXT("CRITICAL_ISSUES");
    }
}