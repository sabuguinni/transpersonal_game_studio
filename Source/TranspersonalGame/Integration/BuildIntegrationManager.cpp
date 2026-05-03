#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    // Configuração inicial
    BuildVersion = TEXT("1.0.0");
    LastBuildTime = FDateTime::Now();
    bIsValidBuild = false;
    
    // Inicializar contadores
    TotalModules = 0;
    CompiledModules = 0;
    FailedModules = 0;
    
    // Configurar validação
    bValidateOnStartup = true;
    bAutoFixErrors = false;
    MaxValidationTime = 30.0f;
}

void UBuildIntegrationManager::InitializeComponent()
{
    Super::InitializeComponent();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
    
    if (bValidateOnStartup)
    {
        // Agendar validação para próximo tick
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBuildIntegrationManager::ValidateBuildIntegrity);
    }
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Registar este manager globalmente
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        // Guardar referência no GameInstance para acesso global
        GameInstance->GetSubsystem<UBuildIntegrationManager>();
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager started - Version: %s"), *BuildVersion);
}

bool UBuildIntegrationManager::ValidateBuildIntegrity()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Starting build integrity validation..."));
    
    float StartTime = FPlatformTime::Seconds();
    bool bValidationSuccess = true;
    
    // Reset contadores
    TotalModules = 0;
    CompiledModules = 0;
    FailedModules = 0;
    ValidationErrors.Empty();
    
    // 1. Verificar módulos core
    bValidationSuccess &= ValidateCoreModules();
    
    // 2. Verificar dependências
    bValidationSuccess &= ValidateModuleDependencies();
    
    // 3. Verificar assets críticos
    bValidationSuccess &= ValidateCriticalAssets();
    
    // 4. Verificar configurações
    bValidationSuccess &= ValidateGameConfiguration();
    
    float EndTime = FPlatformTime::Seconds();
    float ValidationTime = EndTime - StartTime;
    
    bIsValidBuild = bValidationSuccess;
    LastValidationTime = ValidationTime;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Build validation completed in %.2fs - Success: %s"), 
           ValidationTime, bValidationSuccess ? TEXT("YES") : TEXT("NO"));
    
    if (!bValidationSuccess)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Build validation failed with %d errors"), ValidationErrors.Num());
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("  - %s"), *Error);
        }
        
        if (bAutoFixErrors)
        {
            AttemptAutoFix();
        }
    }
    
    // Broadcast resultado
    OnBuildValidationComplete.Broadcast(bValidationSuccess, ValidationErrors);
    
    return bValidationSuccess;
}

bool UBuildIntegrationManager::ValidateCoreModules()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating core modules..."));
    
    bool bSuccess = true;
    TotalModules = 0;
    CompiledModules = 0;
    
    // Lista de módulos críticos que devem existir
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Engine"),
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("UnrealEd")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        TotalModules++;
        
        // Verificar se módulo está carregado
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            CompiledModules++;
            UE_LOG(LogBuildIntegration, Verbose, TEXT("Module loaded: %s"), *ModuleName);
        }
        else
        {
            FailedModules++;
            FString Error = FString::Printf(TEXT("Core module not loaded: %s"), *ModuleName);
            ValidationErrors.Add(Error);
            bSuccess = false;
            UE_LOG(LogBuildIntegration, Error, TEXT("%s"), *Error);
        }
    }
    
    return bSuccess;
}

bool UBuildIntegrationManager::ValidateModuleDependencies()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating module dependencies..."));
    
    bool bSuccess = true;
    
    // Verificar dependências críticas do TranspersonalGame
    TArray<FString> RequiredDependencies = {
        TEXT("Engine"),
        TEXT("CoreUObject"),
        TEXT("Slate"),
        TEXT("SlateCore"),
        TEXT("UMG"),
        TEXT("AIModule"),
        TEXT("NavigationSystem"),
        TEXT("Niagara"),
        TEXT("Chaos")
    };
    
    for (const FString& Dependency : RequiredDependencies)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*Dependency))
        {
            FString Error = FString::Printf(TEXT("Required dependency not loaded: %s"), *Dependency);
            ValidationErrors.Add(Error);
            bSuccess = false;
        }
    }
    
    return bSuccess;
}

bool UBuildIntegrationManager::ValidateCriticalAssets()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating critical assets..."));
    
    bool bSuccess = true;
    
    // Verificar se MinPlayableMap existe
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    if (!FPaths::FileExists(FPaths::ProjectContentDir() + TEXT("Maps/MinPlayableMap.umap")))
    {
        FString Error = TEXT("Critical asset missing: MinPlayableMap");
        ValidationErrors.Add(Error);
        bSuccess = false;
    }
    
    // Verificar GameMode
    UClass* GameModeClass = LoadClass<AGameModeBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    if (!GameModeClass)
    {
        FString Error = TEXT("Critical class missing: TranspersonalGameMode");
        ValidationErrors.Add(Error);
        bSuccess = false;
    }
    
    // Verificar Character
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        FString Error = TEXT("Critical class missing: TranspersonalCharacter");
        ValidationErrors.Add(Error);
        bSuccess = false;
    }
    
    return bSuccess;
}

bool UBuildIntegrationManager::ValidateGameConfiguration()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating game configuration..."));
    
    bool bSuccess = true;
    
    // Verificar configurações básicas do projecto
    if (GetWorld())
    {
        AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
        if (!GameMode)
        {
            FString Error = TEXT("No GameMode set for current world");
            ValidationErrors.Add(Error);
            bSuccess = false;
        }
    }
    
    return bSuccess;
}

void UBuildIntegrationManager::AttemptAutoFix()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Attempting auto-fix of validation errors..."));
    
    // Por agora, apenas log dos erros
    // Auto-fix pode ser implementado no futuro
    for (const FString& Error : ValidationErrors)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Auto-fix needed for: %s"), *Error);
    }
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Generating build report..."));
    
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("=== BUILD INTEGRATION REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Build Version: %s\n"), *BuildVersion);
    ReportContent += FString::Printf(TEXT("Build Time: %s\n"), *LastBuildTime.ToString());
    ReportContent += FString::Printf(TEXT("Validation Time: %.2fs\n"), LastValidationTime);
    ReportContent += FString::Printf(TEXT("Build Valid: %s\n"), bIsValidBuild ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("\n"));
    
    ReportContent += FString::Printf(TEXT("=== MODULE STATUS ===\n"));
    ReportContent += FString::Printf(TEXT("Total Modules: %d\n"), TotalModules);
    ReportContent += FString::Printf(TEXT("Compiled Modules: %d\n"), CompiledModules);
    ReportContent += FString::Printf(TEXT("Failed Modules: %d\n"), FailedModules);
    ReportContent += FString::Printf(TEXT("\n"));
    
    if (ValidationErrors.Num() > 0)
    {
        ReportContent += FString::Printf(TEXT("=== VALIDATION ERRORS ===\n"));
        for (int32 i = 0; i < ValidationErrors.Num(); i++)
        {
            ReportContent += FString::Printf(TEXT("%d. %s\n"), i + 1, *ValidationErrors[i]);
        }
    }
    
    // Guardar relatório
    FString ReportPath = FPaths::ProjectLogDir() + TEXT("BuildIntegrationReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Build report saved to: %s"), *ReportPath);
}

FString UBuildIntegrationManager::GetBuildStatus() const
{
    if (bIsValidBuild)
    {
        return FString::Printf(TEXT("VALID - %d/%d modules OK"), CompiledModules, TotalModules);
    }
    else
    {
        return FString::Printf(TEXT("INVALID - %d errors, %d failed modules"), ValidationErrors.Num(), FailedModules);
    }
}

void UBuildIntegrationManager::SetBuildVersion(const FString& NewVersion)
{
    BuildVersion = NewVersion;
    LastBuildTime = FDateTime::Now();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Build version updated to: %s"), *BuildVersion);
}

bool UBuildIntegrationManager::IsModuleLoaded(const FString& ModuleName) const
{
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

void UBuildIntegrationManager::ForceRevalidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Forcing build revalidation..."));
    ValidateBuildIntegrity();
}