#include "VFX_ImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Configurar root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Configurar valores padrão
    MaxActiveEffects = 20.0f;
    bDebugMode = false;
    
    // Inicializar array de efeitos de impacto
    ImpactEffects.SetNum(5);
    
    // Configurar efeito de pegada de dinossauro
    ImpactEffects[0].ImpactType = EVFX_ImpactType::DinosaurFootstep;
    ImpactEffects[0].Scale = FVector(2.0f, 2.0f, 1.0f);
    ImpactEffects[0].Duration = 3.0f;
    ImpactEffects[0].bAttachToActor = false;
    
    // Configurar efeito de impacto de arma
    ImpactEffects[1].ImpactType = EVFX_ImpactType::WeaponHit;
    ImpactEffects[1].Scale = FVector(0.5f, 0.5f, 0.5f);
    ImpactEffects[1].Duration = 1.5f;
    ImpactEffects[1].bAttachToActor = false;
    
    // Configurar efeito de impacto de rocha
    ImpactEffects[2].ImpactType = EVFX_ImpactType::RockImpact;
    ImpactEffects[2].Scale = FVector(1.0f, 1.0f, 1.0f);
    ImpactEffects[2].Duration = 2.0f;
    ImpactEffects[2].bAttachToActor = false;
    
    // Configurar efeito de sangue
    ImpactEffects[3].ImpactType = EVFX_ImpactType::BloodSplatter;
    ImpactEffects[3].Scale = FVector(1.0f, 1.0f, 1.0f);
    ImpactEffects[3].Duration = 4.0f;
    ImpactEffects[3].bAttachToActor = true;
    
    // Configurar efeito de nuvem de poeira
    ImpactEffects[4].ImpactType = EVFX_ImpactType::DustCloud;
    ImpactEffects[4].Scale = FVector(3.0f, 3.0f, 2.0f);
    ImpactEffects[4].Duration = 5.0f;
    ImpactEffects[4].bAttachToActor = false;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: BeginPlay - Manager initialized with %d impact effects"), ImpactEffects.Num());
    }
    
    // Limpar array de efeitos activos
    ActiveEffects.Empty();
}

void UVFX_ImpactManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation, AActor* AttachActor)
{
    // Verificar limite de efeitos activos
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        
        // Se ainda estamos no limite, remover o efeito mais antigo
        if (ActiveEffects.Num() >= MaxActiveEffects && ActiveEffects.Num() > 0)
        {
            UNiagaraComponent* OldestEffect = ActiveEffects[0];
            if (IsValid(OldestEffect))
            {
                OldestEffect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(0);
        }
    }
    
    // Encontrar dados do efeito
    FVFX_ImpactData* EffectData = nullptr;
    for (FVFX_ImpactData& Data : ImpactEffects)
    {
        if (Data.ImpactType == ImpactType)
        {
            EffectData = &Data;
            break;
        }
    }
    
    if (!EffectData)
    {
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No effect data found for impact type"));
        }
        return;
    }
    
    // Verificar se temos sistema Niagara válido
    UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        // Tentar usar sistema padrão de partículas se disponível
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No Niagara system assigned, using fallback"));
        }
        
        // Criar efeito visual simples como fallback
        SpawnDustCloud(Location, 1.0f);
        return;
    }
    
    // Spawnar efeito Niagara
    UNiagaraComponent* EffectComponent = nullptr;
    
    if (EffectData->bAttachToActor && AttachActor)
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EffectData->Scale,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation,
            EffectData->Scale,
            true
        );
    }
    
    if (EffectComponent)
    {
        RegisterImpactEffect(EffectComponent);
        
        // Configurar timer para limpeza automática
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimer,
            [this, EffectComponent]()
            {
                if (IsValid(EffectComponent))
                {
                    UnregisterImpactEffect(EffectComponent);
                    EffectComponent->DestroyComponent();
                }
            },
            EffectData->Duration,
            false
        );
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Spawned effect at location %s"), *Location.ToString());
        }
    }
}

void UVFX_ImpactManager::SpawnDinosaurFootstep(FVector Location, float DinosaurSize)
{
    // Ajustar localização para o chão
    FVector GroundLocation = Location;
    GroundLocation.Z -= 50.0f; // Offset para garantir que está no chão
    
    // Ajustar rotação para apontar para cima
    FRotator UpwardRotation = FRotator(-90.0f, 0.0f, 0.0f);
    
    SpawnImpactEffect(EVFX_ImpactType::DinosaurFootstep, GroundLocation, UpwardRotation);
    
    // Adicionar nuvem de poeira adicional para dinossauros grandes
    if (DinosaurSize > 2.0f)
    {
        SpawnDustCloud(GroundLocation, DinosaurSize * 0.5f);
    }
}

void UVFX_ImpactManager::SpawnBloodEffect(FVector Location, FVector Direction)
{
    // Calcular rotação baseada na direcção
    FRotator BloodRotation = Direction.Rotation();
    
    SpawnImpactEffect(EVFX_ImpactType::BloodSplatter, Location, BloodRotation);
}

void UVFX_ImpactManager::SpawnDustCloud(FVector Location, float Intensity)
{
    FVector DustLocation = Location;
    DustLocation.Z += 10.0f; // Elevar ligeiramente do chão
    
    // Criar efeito de poeira simples usando spawn de partículas
    SpawnImpactEffect(EVFX_ImpactType::DustCloud, DustLocation);
}

void UVFX_ImpactManager::CleanupExpiredEffects()
{
    // Remover componentes inválidos ou destruídos
    ActiveEffects.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });
}

void UVFX_ImpactManager::OnEffectFinished(UNiagaraComponent* FinishedComponent)
{
    if (IsValid(FinishedComponent))
    {
        UnregisterImpactEffect(FinishedComponent);
        FinishedComponent->DestroyComponent();
    }
}

void UVFX_ImpactManager::RegisterImpactEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveEffects.Add(Effect);
    }
}

void UVFX_ImpactManager::UnregisterImpactEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveEffects.Remove(Effect);
    }
}