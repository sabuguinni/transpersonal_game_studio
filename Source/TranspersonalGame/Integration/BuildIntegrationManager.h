#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Agente #19
 * Responsável por integrar outputs de todos os agentes numa build coerente
 * Valida compilação, resolve conflitos, mantém builds funcionais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === VALIDATION SYSTEM ===
    
    /** Validar estado de compilação do projeto */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool ValidateCompilationState();
    
    /** Verificar headers órfãos (sem .cpp correspondente) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    int32 CheckOrphanHeaders();
    
    /** Validar integridade do MinPlayableMap */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool ValidateMapIntegrity();
    
    /** Limpar actores duplicados de lighting */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    void CleanDuplicateLightingActors();

    // === BUILD MANAGEMENT ===
    
    /** Executar build completo via UnrealBuildTool */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool ExecuteFullBuild();
    
    /** Criar backup da build actual */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool CreateBuildBackup();
    
    /** Restaurar última build funcional */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool RestoreLastWorkingBuild();

    // === INTEGRATION VALIDATION ===
    
    /** Verificar compatibilidade entre sistemas */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool ValidateSystemCompatibility();
    
    /** Resolver conflitos de dependências */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    bool ResolveDependencyConflicts();
    
    /** Gerar relatório de integração */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    FString GenerateIntegrationReport();

protected:
    // === PROPERTIES ===
    
    /** Estado actual da build */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build State")
    EBuildState CurrentBuildState;
    
    /** Número de headers órfãos detectados */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build State")
    int32 OrphanHeaderCount;
    
    /** Número de erros de compilação */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build State")
    int32 CompilationErrorCount;
    
    /** Timestamp da última validação */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build State")
    FDateTime LastValidationTime;
    
    /** Lista de sistemas críticos a validar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalSystems;
    
    /** Directório de backups */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString BackupDirectory;
    
    /** Máximo de backups a manter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxBackupsToKeep;

private:
    // === INTERNAL METHODS ===
    
    /** Verificar se uma classe C++ existe e é carregável */
    bool ValidateClass(const FString& ClassName);
    
    /** Contar ficheiros por extensão numa directoria */
    int32 CountFilesByExtension(const FString& Directory, const FString& Extension);
    
    /** Executar comando de sistema e capturar output */
    bool ExecuteSystemCommand(const FString& Command, FString& Output);
    
    /** Limpar ficheiros temporários de build */
    void CleanTemporaryBuildFiles();
    
    /** Validar estrutura de directorias do projeto */
    bool ValidateProjectStructure();
};