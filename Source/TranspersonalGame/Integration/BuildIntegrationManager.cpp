#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar estado de integração
    IntegrationState = EBuild_IntegrationState::Idle;
    LastValidationTime = FDateTime::MinValue();
    ValidationIntervalSeconds = 300.0f; // 5 minutos
    
    // Configurar validações críticas
    CriticalValidations.Add(TEXT("ModuleCompilation"));
    CriticalValidations.Add(TEXT("ActorSpawning"));
    CriticalValidations.Add(TEXT("BiomeDistribution"));
    CriticalValidations.Add(TEXT("LightingSetup"));
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager inicializado"));
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Executar validação inicial
    PerformIntegrationValidation();
}

bool UBuildIntegrationManager::ValidateModuleIntegrity()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validando integridade dos módulos..."));
    
    // Verificar se classes críticas estão carregadas
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    int32 LoadedClasses = 0;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogBuildIntegration, Log, TEXT("✅ Classe carregada: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("❌ Falha ao carregar: %s"), *ClassName);
        }
    }
    
    bool bModulesValid = (LoadedClasses == CriticalClasses.Num());
    UE_LOG(LogBuildIntegration, Log, TEXT("Módulos válidos: %d/%d"), LoadedClasses, CriticalClasses.Num());
    
    return bModulesValid;
}

bool UBuildIntegrationManager::ValidateMapActors()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validando actores do MinPlayableMap..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Mundo não encontrado"));
        return false;
    }
    
    // Contar actores por tipo
    int32 LightingActors = 0;
    int32 TerrainActors = 0;
    int32 DinosaurActors = 0;
    int32 PlayerStarts = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ActorClass = Actor->GetClass()->GetName();
        
        if (ActorClass.Contains(TEXT("Light")) || ActorClass.Contains(TEXT("Sky")))
        {
            LightingActors++;
        }
        else if (ActorClass.Contains(TEXT("Landscape")) || ActorClass.Contains(TEXT("Terrain")))
        {
            TerrainActors++;
        }
        else if (ActorClass.Contains(TEXT("Dinosaur")) || ActorClass.Contains(TEXT("Pawn")))
        {
            DinosaurActors++;
        }
        else if (ActorClass.Contains(TEXT("PlayerStart")))
        {
            PlayerStarts++;
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Actores de iluminação: %d"), LightingActors);
    UE_LOG(LogBuildIntegration, Log, TEXT("Actores de terreno: %d"), TerrainActors);
    UE_LOG(LogBuildIntegration, Log, TEXT("Dinossauros: %d"), DinosaurActors);
    UE_LOG(LogBuildIntegration, Log, TEXT("PlayerStarts: %d"), PlayerStarts);
    
    // Validar requisitos mínimos
    bool bMapValid = (LightingActors > 0 && TerrainActors > 0 && PlayerStarts > 0);
    
    if (bMapValid)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("✅ MinPlayableMap válido"));
    }
    else
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("❌ MinPlayableMap inválido - faltam actores críticos"));
    }
    
    return bMapValid;
}

bool UBuildIntegrationManager::ValidateBiomeDistribution()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validando distribuição de biomas..."));
    
    // Coordenadas dos 5 biomas conforme memória crítica
    TArray<FBuild_BiomeZone> BiomeZones = {
        {TEXT("Pantano"), FVector(-50000, -45000, 0), FVector2D(-77500, -25000), FVector2D(-76500, -15000)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0), FVector2D(-77500, -15000), FVector2D(15000, 76500)},
        {TEXT("Savana"), FVector(0, 0, 0), FVector2D(-20000, 20000), FVector2D(-20000, 20000)},
        {TEXT("Deserto"), FVector(55000, 0, 0), FVector2D(25000, 79500), FVector2D(-30000, 30000)},
        {TEXT("Montanha"), FVector(40000, 50000, 500), FVector2D(15000, 79500), FVector2D(20000, 76500)}
    };
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Verificar se existem actores em cada bioma
    int32 ValidBiomes = 0;
    
    for (const FBuild_BiomeZone& Biome : BiomeZones)
    {
        int32 ActorsInBiome = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            FVector ActorLocation = Actor->GetActorLocation();
            
            // Verificar se actor está dentro da zona do bioma
            if (ActorLocation.X >= Biome.XRange.X && ActorLocation.X <= Biome.XRange.Y &&
                ActorLocation.Y >= Biome.YRange.X && ActorLocation.Y <= Biome.YRange.Y)
            {
                ActorsInBiome++;
            }
        }
        
        if (ActorsInBiome > 0)
        {
            ValidBiomes++;
            UE_LOG(LogBuildIntegration, Log, TEXT("✅ Bioma %s: %d actores"), *Biome.Name, ActorsInBiome);
        }
        else
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("⚠️ Bioma %s: sem actores"), *Biome.Name);
        }
    }
    
    bool bBiomesValid = (ValidBiomes >= 3); // Pelo menos 3 biomas devem ter actores
    UE_LOG(LogBuildIntegration, Log, TEXT("Biomas válidos: %d/5"), ValidBiomes);
    
    return bBiomesValid;
}

void UBuildIntegrationManager::PerformIntegrationValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== VALIDAÇÃO DE INTEGRAÇÃO INICIADA ==="));
    
    IntegrationState = EBuild_IntegrationState::Validating;
    
    // Executar todas as validações críticas
    bool bModulesValid = ValidateModuleIntegrity();
    bool bMapValid = ValidateMapActors();
    bool bBiomesValid = ValidateBiomeDistribution();
    
    // Determinar estado final
    if (bModulesValid && bMapValid && bBiomesValid)
    {
        IntegrationState = EBuild_IntegrationState::Healthy;
        UE_LOG(LogBuildIntegration, Log, TEXT("✅ INTEGRAÇÃO SAUDÁVEL - Todas as validações passaram"));
    }
    else
    {
        IntegrationState = EBuild_IntegrationState::Error;
        UE_LOG(LogBuildIntegration, Error, TEXT("❌ INTEGRAÇÃO COM PROBLEMAS"));
        
        if (!bModulesValid) UE_LOG(LogBuildIntegration, Error, TEXT("  - Módulos com problemas"));
        if (!bMapValid) UE_LOG(LogBuildIntegration, Error, TEXT("  - Mapa com problemas"));
        if (!bBiomesValid) UE_LOG(LogBuildIntegration, Error, TEXT("  - Biomas com problemas"));
    }
    
    LastValidationTime = FDateTime::Now();
    
    // Gerar relatório de integração
    GenerateIntegrationReport();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("=== VALIDAÇÃO DE INTEGRAÇÃO CONCLUÍDA ==="));
}

void UBuildIntegrationManager::GenerateIntegrationReport()
{
    FString ReportContent = TEXT("=== RELATÓRIO DE INTEGRAÇÃO ===\n");
    ReportContent += FString::Printf(TEXT("Data: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Estado: %s\n"), 
        IntegrationState == EBuild_IntegrationState::Healthy ? TEXT("SAUDÁVEL") : TEXT("COM PROBLEMAS"));
    
    ReportContent += TEXT("\nValidações executadas:\n");
    for (const FString& Validation : CriticalValidations)
    {
        ReportContent += FString::Printf(TEXT("- %s\n"), *Validation);
    }
    
    // Guardar relatório em ficheiro
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("IntegrationReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Relatório guardado em: %s"), *ReportPath);
}

EBuild_IntegrationState UBuildIntegrationManager::GetIntegrationState() const
{
    return IntegrationState;
}

bool UBuildIntegrationManager::IsValidationRequired() const
{
    FTimespan TimeSinceValidation = FDateTime::Now() - LastValidationTime;
    return TimeSinceValidation.GetTotalSeconds() > ValidationIntervalSeconds;
}