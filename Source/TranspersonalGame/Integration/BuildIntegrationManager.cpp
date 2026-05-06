#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar estado de integração
    IntegrationState = EBuild_IntegrationState::Initializing;
    LastValidationTime = 0.0f;
    ValidationInterval = 30.0f; // Validar a cada 30 segundos
    
    // Configurar contadores
    TotalModulesLoaded = 0;
    FailedModulesCount = 0;
    OrphanHeadersCount = 0;
    DuplicateActorsCount = 0;
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager iniciado"));
    
    // Executar validação inicial
    PerformIntegrationValidation();
    
    // Configurar timer para validações periódicas
    GetWorld()->GetTimerManager().SetTimer(
        ValidationTimerHandle,
        this,
        &UBuildIntegrationManager::PerformIntegrationValidation,
        ValidationInterval,
        true
    );
}

void UBuildIntegrationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Limpar timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UBuildIntegrationManager::PerformIntegrationValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Executando validação de integração..."));
    
    IntegrationState = EBuild_IntegrationState::Validating;
    
    // 1. Validar módulos C++ carregados
    ValidateLoadedModules();
    
    // 2. Verificar headers órfãos
    CheckOrphanHeaders();
    
    // 3. Verificar actores duplicados
    CheckDuplicateActors();
    
    // 4. Validar dependências entre sistemas
    ValidateSystemDependencies();
    
    // 5. Atualizar estado final
    UpdateIntegrationState();
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Validação concluída - Estado: %s"), 
           *UEnum::GetValueAsString(IntegrationState));
}

void UBuildIntegrationManager::ValidateLoadedModules()
{
    TotalModulesLoaded = 0;
    FailedModulesCount = 0;
    
    // Lista de classes críticas que devem estar carregadas
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
    
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            TotalModulesLoaded++;
            UE_LOG(LogBuildIntegration, Log, TEXT("Módulo carregado: %s"), *ClassName);
        }
        else
        {
            FailedModulesCount++;
            UE_LOG(LogBuildIntegration, Error, TEXT("Falha ao carregar módulo: %s"), *ClassName);
        }
    }
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Módulos: %d carregados, %d falharam"), 
           TotalModulesLoaded, FailedModulesCount);
}

void UBuildIntegrationManager::CheckOrphanHeaders()
{
    OrphanHeadersCount = 0;
    
    // Obter caminho do projeto
    FString ProjectPath = FPaths::ProjectDir();
    FString SourcePath = FPaths::Combine(ProjectPath, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Encontrar todos os ficheiros .h
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
    
    // Verificar se cada .h tem .cpp correspondente
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            OrphanHeadersCount++;
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *ProjectPath);
            UE_LOG(LogBuildIntegration, Warning, TEXT("Header órfão: %s"), *RelativePath);
        }
    }
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Headers órfãos encontrados: %d"), OrphanHeadersCount);
}

void UBuildIntegrationManager::CheckDuplicateActors()
{
    DuplicateActorsCount = 0;
    
    if (!GetWorld())
    {
        return;
    }
    
    // Contar actores de lighting duplicados
    TArray<AActor*> DirectionalLights;
    TArray<AActor*> SkyLights;
    TArray<AActor*> SkyAtmospheres;
    TArray<AActor*> HeightFogs;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            DirectionalLights.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("SkyLight")))
        {
            SkyLights.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            SkyAtmospheres.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("ExponentialHeightFog")))
        {
            HeightFogs.Add(Actor);
        }
    }
    
    // Contar duplicados (deveria haver apenas 1 de cada)
    if (DirectionalLights.Num() > 1) DuplicateActorsCount += DirectionalLights.Num() - 1;
    if (SkyLights.Num() > 1) DuplicateActorsCount += SkyLights.Num() - 1;
    if (SkyAtmospheres.Num() > 1) DuplicateActorsCount += SkyAtmospheres.Num() - 1;
    if (HeightFogs.Num() > 1) DuplicateActorsCount += HeightFogs.Num() - 1;
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Actores duplicados: %d"), DuplicateActorsCount);
}

void UBuildIntegrationManager::ValidateSystemDependencies()
{
    // Verificar se sistemas críticos estão funcionais
    bool bWorldGenValid = ValidateWorldGeneration();
    bool bCharacterValid = ValidateCharacterSystem();
    bool bGameModeValid = ValidateGameMode();
    
    SystemValidationResults.Empty();
    SystemValidationResults.Add(TEXT("WorldGeneration"), bWorldGenValid);
    SystemValidationResults.Add(TEXT("Character"), bCharacterValid);
    SystemValidationResults.Add(TEXT("GameMode"), bGameModeValid);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação de sistemas:"));
    UE_LOG(LogBuildIntegration, Log, TEXT("- WorldGeneration: %s"), bWorldGenValid ? TEXT("OK") : TEXT("FAIL"));
    UE_LOG(LogBuildIntegration, Log, TEXT("- Character: %s"), bCharacterValid ? TEXT("OK") : TEXT("FAIL"));
    UE_LOG(LogBuildIntegration, Log, TEXT("- GameMode: %s"), bGameModeValid ? TEXT("OK") : TEXT("FAIL"));
}

bool UBuildIntegrationManager::ValidateWorldGeneration()
{
    // Verificar se PCGWorldGenerator está carregado
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    return WorldGenClass != nullptr;
}

bool UBuildIntegrationManager::ValidateCharacterSystem()
{
    // Verificar se TranspersonalCharacter está carregado
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    return CharacterClass != nullptr;
}

bool UBuildIntegrationManager::ValidateGameMode()
{
    // Verificar se TranspersonalGameMode está carregado
    UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    return GameModeClass != nullptr;
}

void UBuildIntegrationManager::UpdateIntegrationState()
{
    // Determinar estado baseado nos resultados da validação
    if (FailedModulesCount > 0)
    {
        IntegrationState = EBuild_IntegrationState::Failed;
    }
    else if (OrphanHeadersCount > 50) // Limite crítico de headers órfãos
    {
        IntegrationState = EBuild_IntegrationState::Warning;
    }
    else if (DuplicateActorsCount > 10) // Limite crítico de duplicados
    {
        IntegrationState = EBuild_IntegrationState::Warning;
    }
    else
    {
        IntegrationState = EBuild_IntegrationState::Healthy;
    }
}

FString UBuildIntegrationManager::GetIntegrationReport() const
{
    FString Report = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Estado: %s\n"), *UEnum::GetValueAsString(IntegrationState));
    Report += FString::Printf(TEXT("Módulos carregados: %d\n"), TotalModulesLoaded);
    Report += FString::Printf(TEXT("Módulos falharam: %d\n"), FailedModulesCount);
    Report += FString::Printf(TEXT("Headers órfãos: %d\n"), OrphanHeadersCount);
    Report += FString::Printf(TEXT("Actores duplicados: %d\n"), DuplicateActorsCount);
    Report += FString::Printf(TEXT("Última validação: %.2f segundos atrás\n"), 
              GetWorld() ? GetWorld()->GetTimeSeconds() - LastValidationTime : 0.0f);
    
    Report += TEXT("\nSistemas validados:\n");
    for (const auto& Result : SystemValidationResults)
    {
        Report += FString::Printf(TEXT("- %s: %s\n"), 
                  *Result.Key, Result.Value ? TEXT("OK") : TEXT("FAIL"));
    }
    
    return Report;
}

void UBuildIntegrationManager::ForceValidation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Validação forçada pelo utilizador"));
    PerformIntegrationValidation();
}

void UBuildIntegrationManager::SetValidationInterval(float NewInterval)
{
    ValidationInterval = FMath::Clamp(NewInterval, 10.0f, 300.0f); // Entre 10s e 5min
    
    // Reiniciar timer com novo intervalo
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformIntegrationValidation,
            ValidationInterval,
            true
        );
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Intervalo de validação alterado para %.1f segundos"), ValidationInterval);
}