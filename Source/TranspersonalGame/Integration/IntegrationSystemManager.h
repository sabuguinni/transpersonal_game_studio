#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "IntegrationSystemManager.generated.h"

/**
 * Integration System Manager
 * Subsistema global que coordena a integração entre todos os módulos do jogo.
 * Responsável por:
 * - Monitorizar o estado de todos os sistemas
 * - Detectar conflitos entre módulos
 * - Validar dependências entre sistemas
 * - Gerar relatórios de saúde do sistema
 * - Coordenar a inicialização ordenada dos módulos
 */
UCLASS()
class TRANSPERSONALGAME_API UIntegrationSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Obter a instância singleton */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    static UIntegrationSystemManager* Get(const UObject* WorldContext);

    /** Registar um módulo no sistema de integração */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterModule(const FString& ModuleName, UObject* ModuleInstance);

    /** Desregistar um módulo */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterModule(const FString& ModuleName);

    /** Verificar se um módulo está registado */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    bool IsModuleRegistered(const FString& ModuleName) const;

    /** Obter instância de um módulo registado */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    UObject* GetModuleInstance(const FString& ModuleName) const;

    /** Validar todas as dependências entre módulos */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleDependencies();

    /** Gerar relatório completo do sistema */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateSystemReport();

    /** Verificar se o sistema está saudável */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Integration")
    bool IsSystemHealthy() const;

    /** Inicializar todos os módulos na ordem correcta */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeAllModules();

    /** Shutdown de todos os módulos */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ShutdownAllModules();

protected:
    /** Validar um módulo específico */
    bool ValidateModule(const FString& ModuleName, UObject* ModuleInstance);

    /** Verificar dependências de um módulo */
    bool CheckModuleDependencies(const FString& ModuleName);

    /** Ordenar módulos por dependências */
    TArray<FString> GetModuleInitializationOrder();

public:
    /** Mapa de módulos registados */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, UObject*> RegisteredModules;

    /** Lista de módulos que falharam a validação */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedModules;

    /** Lista de dependências não satisfeitas */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> UnsatisfiedDependencies;

    /** Se o sistema foi inicializado com sucesso */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bSystemInitialized;

    /** Timestamp da última validação */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

    /** Número total de módulos registados */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalRegisteredModules;

    /** Número de módulos activos */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveModulesCount;
};